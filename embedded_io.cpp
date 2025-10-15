#include "embedded_io.h"
#include <QDebug>
#include <QProcess>
#include "file_handler.h"
#include <QRegularExpression>

// ===== sysfs helpers =====
static QString gpioPath(int n, const char* leaf=nullptr){
    QString p = QString("/sys/class/gpio/gpio%1").arg(n);
    if (leaf) p += "/" + QString::fromLatin1(leaf);
    return p;
}
bool EmbeddedIO::exportGpio(int n){
    if (n < 0) return true;
    if (QFile::exists(gpioPath(n).toUtf8())) return true;
    QFile f("/sys/class/gpio/export");
    if (!f.open(QIODevice::WriteOnly|QIODevice::Text)) return false;
    QTextStream out(&f); out << n; return true;
}
bool EmbeddedIO::unexportGpio(int n){
    if (n < 0) return true;
    if (!QFile::exists(gpioPath(n).toUtf8())) return true;
    QFile f("/sys/class/gpio/unexport");
    if (!f.open(QIODevice::WriteOnly|QIODevice::Text)) return false;
    QTextStream out(&f); out << n; return true;
}
bool EmbeddedIO::setDir(int n, const char* dir){
    if (n < 0) return true;
    QFile f(gpioPath(n, "direction"));
    if (!f.open(QIODevice::WriteOnly|QIODevice::Text)) return false;
    QTextStream out(&f); out << dir; return true;
}
bool EmbeddedIO::setEdge(int n, const char* edge){
    if (n < 0) return true;
    QFile f(gpioPath(n, "edge"));
    if (!f.open(QIODevice::WriteOnly|QIODevice::Text)) return false;
    QTextStream out(&f); out << edge; return true;
}
bool EmbeddedIO::writeVal(int n, int v){
    if (n < 0) return true;
    QFile f(gpioPath(n, "value"));
    if (!f.open(QIODevice::WriteOnly|QIODevice::Text)) return false;
    QTextStream out(&f); out << (v?1:0); return true;
}

int EmbeddedIO::readVal(int n){
    if (n < 0) return 1;
    QFile f(gpioPath(n, "value"));
    if (!f.open(QIODevice::ReadOnly|QIODevice::Text)) {
        static bool warned=false;
        if (!warned) {
            qWarning() << "[GPIO] No puedo abrir" << gpioPath(n, "value")
            << "¿exportado? ¿permisos? ¿GPIO correcto?";
            warned=true;
        }
        return 1; // fallback
    }
    QByteArray b = f.readAll().trimmed();
    return (b=="1") ? 1 : 0;
}

// ===== class =====
EmbeddedIO::EmbeddedIO(QObject* parent): QObject(parent) {
    initGpios();
    _startMs = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "[BTN] timer cada" << BTN_SAMPLE_MS << "ms";

    // --- Heartbeat GPIO I07 ---
    connect(&_hbTimer, &QTimer::timeout, [this](){
        _linkLevel = !_linkLevel;
        writeVal(GPIO_I07_HEARTBEAT, _linkLevel);
    });
    _hbTimer.start(HB_PERIOD_MS);

    // --- LINK LED parpadeo/estado ---
    connect(&_linkTimer, &QTimer::timeout, this, &EmbeddedIO::updateLinkLed);
    _linkTimer.start(_linkBlinkMs);

    // --- Botón de fábrica (muestreo 20ms + antirrebotes SW) ---
    connect(&_btnTimer, &QTimer::timeout, this, &EmbeddedIO::onFactoryButtonSample);
    _btnTimer.start(BTN_SAMPLE_MS);

    // --- Petición de reset desde MCU (opcional) ---
    if (GPIO_I08_MCU_REQ_RST >= 0) {
        connect(&_mcuReqTimer, &QTimer::timeout, this, &EmbeddedIO::onMcuReqRstSample);
        _mcuReqTimer.start(50);
    }
}

EmbeddedIO::~EmbeddedIO(){
    // Apaga LED y heartbeat al salir
    writeVal(GPIO_I06_LINK_LED, 0);
    writeVal(GPIO_I07_HEARTBEAT, 0);
    // No desexportamos por si otros procesos comparten GPIOs
}

void EmbeddedIO::initGpios(){
    exportGpio(GPIO_I06_LINK_LED);    setDir(GPIO_I06_LINK_LED, "out");
    exportGpio(GPIO_I07_HEARTBEAT);   setDir(GPIO_I07_HEARTBEAT, "out");
    exportGpio(GPIO_I09_BTN_FACTORY); setDir(GPIO_I09_BTN_FACTORY, "in");
    setEdge(GPIO_I09_BTN_FACTORY, "none");
    if (GPIO_I08_MCU_REQ_RST >= 0) { exportGpio(GPIO_I08_MCU_REQ_RST); setDir(GPIO_I08_MCU_REQ_RST, "in"); }

    writeVal(GPIO_I06_LINK_LED, /*valor que apaga*/ 1); // activo-bajo → 1 apaga, 0 enciende

    // Autodetección de polaridad (idle = nivel en reposo)
    _btnIdleLevel = readVal(GPIO_I09_BTN_FACTORY);
    qDebug() << "[BTN] GPIO" << GPIO_I09_BTN_FACTORY
             << "idle=" << _btnIdleLevel
             << "(pressed será" << (_btnIdleLevel? "0" : "1") << ")";
}


void EmbeddedIO::setLinkState(LinkState s){
    _state = s;
    switch (_state){
    case LinkState::Booting:  _linkBlinkMs = BOOT_BLINK_MS; break;
    case LinkState::Ready:    _linkBlinkMs = READY_BLINK_MS; break;
    case LinkState::Degraded: _linkBlinkMs = DEGRADED_BLINK_MS; break;
    }
    _linkTimer.stop();
    if (_linkBlinkMs == 0) { writeVal(GPIO_I06_LINK_LED, 1); }
    else { _linkTimer.start(_linkBlinkMs); }
}

void EmbeddedIO::updateLinkLed(){
    if (_linkBlinkMs == 0) { writeVal(GPIO_I06_LINK_LED, 1); return; }
    static bool lev=false; lev = !lev;
    writeVal(GPIO_I06_LINK_LED, lev);
}

void EmbeddedIO::onFactoryButtonSample(){
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    // Ignora picos de arranque 3 s
    if (now - _startMs < 3000) {
        static bool once=false;
        if (!once) { qDebug() << "[BTN] Ignorando durante warmup (3s)"; once=true; }
        return;
    }

    // Lee crudo y normaliza: "pressed" = (raw != idle)
    const int raw = readVal(GPIO_I09_BTN_FACTORY);     // 0/1 desde sysfs
    const bool pressed = (raw != _btnIdleLevel);
    static int dbgTick = 0;
    if ((dbgTick++ % (250/BTN_SAMPLE_MS)) == 0) {      // cada ~250 ms
        qDebug() << "[BTN] raw=" << raw
                 << "pressed=" << pressed
                 << "accum=" << _btnAccumMs
                 << "rel=" << _btnReleaseMs
                 << "armed=" << _factoryArmed;
    }

    // Armar sólo tras ver suelto 0.8 s (evita disparar si queda pulsado tras reboot)
    if (!_factoryArmed) {
        if (!pressed) {
            _btnReleaseMs += BTN_SAMPLE_MS;
            if (_btnReleaseMs >= 800) {
                _factoryArmed = true;
                _btnAccumMs = 0;
                qDebug() << "[BTN] ARMED";
            }
        } else {
            _btnReleaseMs = 0;
        }
        return;
    }

    // Integración robusta (antirrebotes)
    if (pressed) {
        _btnAccumMs += BTN_SAMPLE_MS;
        if ((_btnAccumMs % 500) == 0) qDebug() << "[BTN] held ms:" << _btnAccumMs;
        if (_btnAccumMs >= FACTORY_HOLD_MS) {
            qDebug() << "[BTN] FACTORY threshold reached -> reset";
            _factoryArmed = false;       // evita reentradas
            _btnTimer.stop();
            _linkBlinkMs = 100; _linkTimer.start(_linkBlinkMs);   // feedback 2 s
            QTimer::singleShot(2000, this, [this](){ doFactoryResetAndReboot(); });
        }
    } else {
        // “descarga” para tolerar rebotes: cae 3x más rápido que sube
        _btnAccumMs = qMax(0, _btnAccumMs - 3*BTN_SAMPLE_MS);
    }
}


void EmbeddedIO::onMcuReqRstSample(){
    if (readVal(GPIO_I08_MCU_REQ_RST)==0){ // nivel bajo = petición
        // Feedback corto
        for (int i=0;i<6;i++){ writeVal(GPIO_I06_LINK_LED, (i&1)); usleep(100*1000); }
        softRebootProcess();
    }
}


// --------- TUS FUNCIONES, ARREGLADAS (mismos nombres) ---------

void EmbeddedIO::setIPAddressFileV2(QString ipAddress)
{
    QFile interfacesFile(INTERFACES_PATH);
    if (!interfacesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "NO SE PUEDE ABRIR INTERFACES (leer)";
        return;
    }
    QStringList lines;
    {
        QTextStream in(&interfacesFile);
        while (!in.atEnd()) lines << in.readLine();
        interfacesFile.close();
    }

    // Reemplaza línea que empiece por "address " si existe; si no, añádela
    bool replaced = false;
    QRegularExpression re(R"(^\s*address\s+)");
    for (int i=0; i<lines.size(); ++i) {
        if (re.match(lines[i]).hasMatch()) { lines[i] = "address " + ipAddress; replaced = true; }
    }
    if (!replaced) lines << ("address " + ipAddress);

    // Escribe truncando (no seek)
    if (!interfacesFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "NO SE PUEDE ABRIR INTERFACES (escribir)";
        return;
    }
    QTextStream out(&interfacesFile); for (const auto& l : lines) out << l << '\n';
    interfacesFile.flush(); interfacesFile.close();
}

void EmbeddedIO::setSubmaskAddressFileV2(QString submaskAddress)
{
    QFile interfacesFile(INTERFACES_PATH);
    if (!interfacesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "NO SE PUEDE ABRIR INTERFACES (leer)";
        return;
    }
    QStringList lines;
    {
        QTextStream in(&interfacesFile);
        while (!in.atEnd()) lines << in.readLine();
        interfacesFile.close();
    }

    bool replaced = false;
    QRegularExpression re(R"(^\s*netmask\s+)");
    for (int i=0; i<lines.size(); ++i) {
        if (re.match(lines[i]).hasMatch()) { lines[i] = "netmask " + submaskAddress; replaced = true; }
    }
    if (!replaced) lines << ("netmask " + submaskAddress);

    if (!interfacesFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "NO SE PUEDE ABRIR INTERFACES (escribir)";
        return;
    }
    QTextStream out(&interfacesFile); for (const auto& l : lines) out << l << '\n';
    interfacesFile.flush(); interfacesFile.close();
}

void EmbeddedIO::setGatewayAddressFileV2(QString gatewayAddress)
{
    QFile interfacesFile(INTERFACES_PATH);
    if (!interfacesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "NO SE PUEDE ABRIR INTERFACES (leer)";
        return;
    }
    QStringList lines;
    {
        QTextStream in(&interfacesFile);
        while (!in.atEnd()) lines << in.readLine();
        interfacesFile.close();
    }

    bool replaced = false;
    QRegularExpression re(R"(^\s*gateway\s+)");
    QString newline = gatewayAddress.isEmpty() ? "gateway 0.0.0.0" : ("gateway " + gatewayAddress);
    for (int i=0; i<lines.size(); ++i) {
        if (re.match(lines[i]).hasMatch()) { lines[i] = newline; replaced = true; }
    }
    if (!replaced) lines << newline;

    if (!interfacesFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qWarning() << "NO SE PUEDE ABRIR INTERFACES (escribir)";
        return;
    }
    QTextStream out(&interfacesFile); for (const auto& l : lines) out << l << '\n';
    interfacesFile.flush(); interfacesFile.close();
}

void EmbeddedIO::setIPConfigInfoV2(QStringList webServerParts, Database* database)
{
    // Asegura 5 campos para tu DB: [ip, mask, gw, building, line]
    while (webServerParts.size() < 5) webServerParts << "NO_NAME";

    const QString ip      = webServerParts.value(0);
    const QString submask = webServerParts.value(1);
    const QString gateway = webServerParts.value(2);

    // 1) SOLO escribe estas líneas con tus setters (sin añadir stanzas nuevas)
    setIPAddressFileV2(ip);
    setSubmaskAddressFileV2(submask);
    setGatewayAddressFileV2(gateway);

    // 2) Guarda en DB (tus 5 parámetros)
    if (database) database->setInterfaceParameters(webServerParts);

    // 3) Aplicar en caliente sin romper ifupdown
    //    - Para DHCP si estuviera activo
    QProcess::execute("sh", {"-c", "pkill -f 'dhclient|udhcpc' 2>/dev/null || true"});

    //    - Baja y limpia estado previo (evita “RTNETLINK: File exists”)
    QProcess::execute("sh", {"-c", QString("ifdown --force %1 2>/dev/null || true").arg(IFACE_NAME)});
    QProcess::execute("sh", {"-c", QString("ip addr flush dev %1").arg(IFACE_NAME)});
    QProcess::execute("sh", {"-c", "ip route del default 2>/dev/null || true"});

    //    - Si existe ifupdown, vuelve a subir con ifup
    const int hasIfup = QProcess::execute("sh", {"-c", "command -v ifup >/dev/null"});
    if (hasIfup == 0) {
        QProcess::execute("sh", {"-c", QString("ifup %1").arg(IFACE_NAME)});
        qInfo() << "[interfaces] recargado con ifupdown (flush+ifup)";
        return;
    }

    //    - Sin ifupdown: aplicar con iproute2 (máscara -> CIDR)
    unsigned a=0,b=0,c=0,d=0; int cidr=24;
    if (std::sscanf(submask.toLatin1().constData(), "%u.%u.%u.%u", &a,&b,&c,&d) == 4) {
        const uint32_t v = (a<<24)|(b<<16)|(c<<8)|d;
        // popcount de la máscara (válida tipo 255.255.255.0 → /24)
        cidr = __builtin_popcount(v);
        if (cidr <= 0 || cidr > 32) cidr = 24;
    }
    const QString ipCidr = ip + "/" + QString::number(cidr);

    QProcess::execute("sh", {"-c", QString("ip link set %1 up").arg(IFACE_NAME)});
    QProcess::execute("sh", {"-c", QString("ip addr add %1 dev %2 2>/dev/null || ip addr replace %1 dev %2")
                                       .arg(ipCidr, IFACE_NAME)});
    if (!gateway.isEmpty() && gateway != "0.0.0.0") {
        QProcess::execute("sh", {"-c", QString("ip route replace default via %1 dev %2")
                                         .arg(gateway, IFACE_NAME)});
    }
    qInfo() << "[interfaces] aplicada ahora" << IFACE_NAME << ipCidr << "gw" << gateway;
}


void EmbeddedIO::doFactoryResetAndReboot()
    // --- 1) Persistente: escribe en tu DB + /etc/network/interfaces usando TU función ---
    {
        _linkBlinkMs = 100;
        _linkTimer.start(_linkBlinkMs);
        Database db;
        db.initDatabase();

        QStringList factory;
        factory << DEFAULT_IP_ADDRESS
                << DEFAULT_SUBMASK_ADDRESS
                << "192.168.1.1"
                << "NO_NAME"
                << "NO_NAME";
        setIPConfigInfoV2(factory, &db);

    rebootDevice();

}

void EmbeddedIO::softRebootProcess(){
    // Reinicia el propio binario (igual que tu watchdog)
    QString program = QCoreApplication::applicationFilePath();
    QByteArray path = program.toLocal8Bit();
    execl(path.constData(), path.constData(), (char*)nullptr);
    _exit(1);
}
