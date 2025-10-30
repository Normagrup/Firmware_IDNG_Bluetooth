#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <thread>     // para std::thread
#include <unistd.h>   // para execl, _exit
#include <chrono>
#include <atomic> // std::atomic<qint64>
#include <cstring>    // strlen (write)
#include <csignal>
#include <fcntl.h>
#include <QProcess>
#include <QFile>

#include "Wireless.h"
#include "embedded_io.h"


// Tiempo máximo permitido sin responder
constexpr int WATCHDOG_TIMEOUT_MS = 12000;

// Guarda el momento desde arranque de la última vez que el hilo principal respondió
static std::atomic<long long> g_lastHeartbeatSteadyMs{0};

static QString i2cgetByte(const QString &regHex)
{
    QProcess p;
    p.start("i2cget", {"-y","1","0x51", regHex, "b"});
    p.waitForFinished(-1);
    if (p.exitStatus()!=QProcess::NormalExit || p.exitCode()!=0)
        return QString();
    return QString::fromLatin1(p.readAllStandardOutput()).trimmed();
}

// Convierte BCD (texto "0x3A") a decimal (58)
static int bcdToDec(const QString &hx)
{
    if (hx.isEmpty()) return -1;
    QString s = hx;
    if (s.startsWith("0x", Qt::CaseInsensitive)) s.remove(0,2);
    bool ok=false; int v = s.toInt(&ok, 16);
    if (!ok) return -1;
    return ((v >> 4) * 10) + (v & 0x0F);
}

// Lee el RTC (direcciones 0x24..0x2A) y devuelve QDateTime en UTC.
static QDateTime readRtcUtc()
{
    const int SS = bcdToDec(i2cgetByte("0x24"));   // segundos
    const int MM = bcdToDec(i2cgetByte("0x25"));   // minutos
    const int HH = bcdToDec(i2cgetByte("0x26"));   // horas
    const int DD = bcdToDec(i2cgetByte("0x27"));   // día
    /* int WK = bcdToDec(i2cgetByte("0x28")); */   // dia de la semana
    const int MO = bcdToDec(i2cgetByte("0x29"));   // mes
    const int YY = bcdToDec(i2cgetByte("0x2A"));   // año (00..99)

    if (SS<0 || MM<0 || HH<0 || DD<1 || MO<1 || YY<0) return QDateTime();

    const int year = 2000 + YY;
    QDate date(year, MO, DD);
    QTime time(HH, MM, SS);

    return QDateTime(date, time, Qt::UTC);
}

// Establece el reloj del sistema desde un QDateTime (en UTC)
static bool setSystemClockUtc(const QDateTime& utc)
{
    if (!utc.isValid()) return false;
    const QString iso = utc.toString("yyyy-MM-dd HH:mm:ss");
    QProcess p;
    p.start("sudo", {"date","-u","-s", iso});
    p.waitForFinished(-1);
    if (p.exitStatus()!=QProcess::NormalExit || p.exitCode()!=0) {
        qWarning() << "date -u -s failed:" << p.readAllStandardError();
        return false;
    }
    return true;
}

// Fuerza sistema ← RTC siempre, sin umbral
static void syncSystemFromRtc()
{
    const QDateTime rtcUtc = readRtcUtc();   
    if (!rtcUtc.isValid()) {
        qWarning() << "RTC: lectura inválida; NO se ha podido sincronizar el sistema.";
        return;
    }

    // Hora del sistema en UTC
    const QDateTime sysUtc = QDateTime::currentDateTimeUtc();

    // Pon siempre el system clock desde el RTC (UTC)
    if (setSystemClockUtc(rtcUtc)) {
        qInfo() << "System clock puesto SIEMPRE desde RTC (UTC):"
                << rtcUtc.toString("yyyy-MM-dd HH:mm:ss");
    } else {
        qWarning() << "Fallo al poner el system clock desde RTC.";
    }
}


static inline long long nowSteadyMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

static void link_off_now() {
    int fd = ::open("/sys/class/gpio/gpio82/value", O_WRONLY);
    if (fd >= 0) {
#ifdef FD_CLOEXEC
        fcntl(fd, F_SETFD, FD_CLOEXEC);  // equivalente a O_CLOEXEC
#endif
        const char one = '1';
        (void)::write(fd, &one, 1);
        ::close(fd);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Apaga LINK al salir
    QObject::connect(&a, &QCoreApplication::aboutToQuit, &link_off_now);
    std::signal(SIGINT,  [](int){ link_off_now(); std::_Exit(130); });
    std::signal(SIGTERM, [](int){ link_off_now(); std::_Exit(143); });

    qDebug() << "App running...";
     
    // Hora al sistema
    syncSystemFromRtc();

    EmbeddedIO io;

    Wireless* wirelessNet = new Wireless(nullptr);
    wirelessNet->setEmbeddedIO(&io);
    wirelessNet->runNetwork();

    qDebug() << "READY";
    io.markReady();

    // 1) Se inicializa y el hilo principal responde cada 2 s
    g_lastHeartbeatSteadyMs.store(nowSteadyMs(), std::memory_order_relaxed);
    QTimer hb;
    QObject::connect(&hb, &QTimer::timeout, [](){
        g_lastHeartbeatSteadyMs.store(nowSteadyMs(), std::memory_order_relaxed);
    });
    hb.start(2000);

    // 2) Watchdog en hilo separado: si no hay latido, reinicia
    std::thread([](){
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            const long long now  = nowSteadyMs();
            const long long last = g_lastHeartbeatSteadyMs.load(std::memory_order_relaxed);

            if (last != 0 && (now - last) > WATCHDOG_TIMEOUT_MS) {
                const char* msg = "Watchdog: sin actividad; reiniciando...\n";
                write(2, msg, strlen(msg));
                // reinicio
                QString program = QCoreApplication::applicationFilePath();
                QByteArray path = program.toLocal8Bit();
                execl(path.constData(), path.constData(), (char*)nullptr);

                _exit(1);
            }
        }
    }).detach();

    return a.exec();
}
