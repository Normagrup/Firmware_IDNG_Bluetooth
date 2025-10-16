#include "embedded_io.h"
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>

void EmbeddedIO::setLinkLed(bool on) { writeVal(GPIO_I06_LINK_LED, on ? 0 : 1); }
void EmbeddedIO::setFailLed(bool on) { writeVal(GPIO_I07_HEARTBEAT, on ? 0 : 1); }

void EmbeddedIO::setLinkMode(LedMode m, int blinkMs) {
    _linkTimer.stop();
    _linkMode = m;

    switch (m) {
    case LedMode::Off:
        setLinkLed(false);
        break;
    case LedMode::On:
        setLinkLed(true);
        break;
    case LedMode::Blink:
        _linkBlinkMs = qMax(100, blinkMs);
        _linkLevel = false;
        setLinkLed(false);
        _linkTimer.start(_linkBlinkMs/2);
        break;
    }
}

void EmbeddedIO::setFailMode(LedMode m, int blinkMs) {
    _failTimer.stop();
    _failMode = m;

    switch (m) {
    case LedMode::Off:
        setFailLed(false);
        break;
    case LedMode::On:
        setFailLed(true);
        break;
    case LedMode::Blink:
        _failBlinkMs = qMax(100, blinkMs);
        _failLevel = false;
        setFailLed(false);
        _failTimer.start(_failBlinkMs/2);
        break;
    }
}

void EmbeddedIO::onLinkTick() {
    if (_linkMode != LedMode::Blink) return;
    _linkLevel = !_linkLevel;
    setLinkLed(_linkLevel);
}

void EmbeddedIO::onFailTick() {
    if (_failMode != LedMode::Blink) return;
    _failLevel = !_failLevel;
    setFailLed(_failLevel);
}

void EmbeddedIO::markBooting(){
    setLinkMode(LedMode::Off);
    setFailMode(LedMode::Off);
}

void EmbeddedIO::markReady(){
    setFailMode(LedMode::Off);
    setLinkMode(LedMode::On);
}

void EmbeddedIO::beginRebootSequence(){

    setLinkMode(LedMode::Off);
    setFailMode(LedMode::Blink, 400);

    _linkTimer.stop();
    _failTimer.stop();

    writeVal(GPIO_I06_LINK_LED, 0);
    writeVal(GPIO_I07_HEARTBEAT, 0);

    rebootDevice();
}

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

EmbeddedIO::EmbeddedIO(QObject* parent): QObject(parent) {
    initGpios();

    setLinkLed(false);
    setFailLed(false);

    // Timers: una única conexión a handlers
    connect(&_linkTimer, &QTimer::timeout, this, &EmbeddedIO::onLinkTick);
    connect(&_failTimer, &QTimer::timeout, this, &EmbeddedIO::onFailTick);

     markBooting();

    // Botón de fábrica
    connect(&_btnTimer, &QTimer::timeout, this, &EmbeddedIO::onFactoryButtonSample);
    _btnTimer.start(BTN_SAMPLE_MS);
}

EmbeddedIO::~EmbeddedIO(){

    _linkTimer.stop();
    _failTimer.stop();
    writeVal(GPIO_I06_LINK_LED, 1);
    writeVal(GPIO_I07_HEARTBEAT, 1);

}

void EmbeddedIO::initGpios(){
    exportGpio(GPIO_I06_LINK_LED);    setDir(GPIO_I06_LINK_LED, "out");
    exportGpio(GPIO_I07_HEARTBEAT);   setDir(GPIO_I07_HEARTBEAT, "out");
    exportGpio(GPIO_I09_BTN_FACTORY); setDir(GPIO_I09_BTN_FACTORY, "in");
    exportGpio(GPIO_I08_TEST);        setDir(GPIO_I08_TEST, "in");
    setEdge(GPIO_I09_BTN_FACTORY, "none");
    writeVal(GPIO_I06_LINK_LED, 1);
    writeVal(GPIO_I07_HEARTBEAT, 1);

    int v = readVal(GPIO_I08_TEST);
    _btnIdleLevel = readVal(GPIO_I09_BTN_FACTORY);

}

void EmbeddedIO::onFactoryButtonSample(){
    const qint64 now = QDateTime::currentMSecsSinceEpoch();

    // Ignora picos de arranque 3 s
    if (now - _startMs < 3000) {
        static bool once=false;
        if (!once) { /*qDebug() << "[BTN] Ignorando durante warmup (3s)"; */once=true; }
        return;
    }

    const int raw = readVal(GPIO_I09_BTN_FACTORY);
    const bool pressed = (raw != _btnIdleLevel);
    static int dbgTick = 0;
    if ((dbgTick++ % (250/BTN_SAMPLE_MS)) == 0) {
        /*qDebug() << "[BTN] raw=" << raw
                 << "pressed=" << pressed
                 << "accum=" << _btnAccumMs
                 << "rel=" << _btnReleaseMs
                 << "armed=" << _factoryArmed;*/
    }

    if (!_factoryArmed) {
        if (!pressed) {
            _btnReleaseMs += BTN_SAMPLE_MS;
            if (_btnReleaseMs >= 800) {
                _factoryArmed = true;
                _btnAccumMs = 0;
                /*qDebug() << "[BTN] ARMED";*/
            }
        } else {
            _btnReleaseMs = 0;
        }
        return;
    }

    if (pressed) {
        _btnAccumMs += BTN_SAMPLE_MS;
        //if ((_btnAccumMs % 500) == 0) /*qDebug() << "[BTN] held ms:" << _btnAccumMs*/;
        if (_btnAccumMs >= FACTORY_HOLD_MS) {
            /*qDebug() << "[BTN] FACTORY threshold reached -> reset";*/
            _factoryArmed = false;       // evita reentradas
            _btnTimer.stop();
            setLinkMode(LedMode::Off);
            setFailMode(LedMode::Blink, 400);

            // Lanza el reset tras un pequeño margen
            QTimer::singleShot(800, this, [this](){ doFactoryResetAndReboot(); });
        }
    }

    // Botón de Test ( GPIO_I08_TEST)
    static int   testLastRaw      = -1;
    static bool  testPrevPressed  = false;
    static qint64 testLastFireMs  = 0;

    const int rawTest = readVal(GPIO_I08_TEST);
    if (rawTest != testLastRaw) {
        qDebug() << "[TEST] gpio" << GPIO_I08_TEST << "change ->" << rawTest;
        testLastRaw = rawTest;
    }

    const bool pressedTest = (rawTest == 0);
    const qint64 now2 = QDateTime::currentMSecsSinceEpoch();

    // flanco + antirrebote 300 ms
    if (pressedTest && !testPrevPressed && (now2 - testLastFireMs > 300)) {
        testLastFireMs = now2;
        qDebug() << "[TEST] SHORT press -> emit testButtonPressed()";
        emit testButtonPressed();
    }
    testPrevPressed = pressedTest;


    }


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

    bool replaced = false;
    QRegularExpression re(R"(^\s*address\s+)");
    for (int i=0; i<lines.size(); ++i) {
        if (re.match(lines[i]).hasMatch()) { lines[i] = "address " + ipAddress; replaced = true; }
    }
    if (!replaced) lines << ("address " + ipAddress);

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
    while (webServerParts.size() < 5) webServerParts << "NO_NAME";

    const QString ip      = webServerParts.value(0);
    const QString submask = webServerParts.value(1);
    const QString gateway = webServerParts.value(2);

    setIPAddressFileV2(ip);
    setSubmaskAddressFileV2(submask);
    setGatewayAddressFileV2(gateway);

    if (database) database->setInterfaceParameters(webServerParts);

    //    - Para DHCP si estuviera activo
    QProcess::execute("sh", {"-c", "pkill -f 'dhclient|udhcpc' 2>/dev/null || true"});

    //    - Baja y limpia estado previo
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
    //escribe en tu DB + /etc/network/interfaces
    {
        Database db;
        db.initDatabase();

        QStringList factory;
        factory << DEFAULT_IP_ADDRESS
                << DEFAULT_SUBMASK_ADDRESS
                << "192.168.1.1"
                << "NO_NAME"
                << "NO_NAME";

        setIPConfigInfoV2(factory, &db);

        beginRebootSequence();

}
