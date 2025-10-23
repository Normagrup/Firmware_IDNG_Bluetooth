#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <thread>     // para std::thread
#include <unistd.h>   // para execl, _exit
#include <atomic> // std::atomic<qint64>
#include <cstring>    // strlen (write)
#include "Wireless.h"

// Tiempo máximo permitido sin responder
constexpr int WATCHDOG_TIMEOUT_MS = 12000;

// Guarda el momento desde arranque de la última vez que el hilo principal respondió
static std::atomic<qint64> g_lastHeartbeatMs{0};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "App running...";

    Wireless* wirelessNet = new Wireless(nullptr);
    wirelessNet->runNetwork();

    // 1) Se inicializa y el hilo principal responde cada 2 s
    g_lastHeartbeatMs.store(QDateTime::currentMSecsSinceEpoch(), std::memory_order_relaxed);
    QTimer hb;
    QObject::connect(&hb, &QTimer::timeout, [](){
        g_lastHeartbeatMs.store(QDateTime::currentMSecsSinceEpoch(), std::memory_order_relaxed);
    });
    hb.start(2000);

    // 2) Watchdog en hilo separado: si no hay latido, reinicia
    std::thread([](){
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));  // chequeo cada 0.5 s
            const qint64 now  = QDateTime::currentMSecsSinceEpoch();
            const qint64 last = g_lastHeartbeatMs.load(std::memory_order_relaxed);
            if (last != 0 && (now - last) > WATCHDOG_TIMEOUT_MS) {
                const char* msg = "Watchdog: sin actividad; reiniciando...\n";
                write(2, msg, strlen(msg));

                // reinicio
                QString program = QCoreApplication::applicationFilePath();
                QByteArray path = program.toLocal8Bit();
                execl(path.constData(), path.constData(), (char*)nullptr);

                // Si execl falla, fuerza salida con error
                _exit(1);
            }
        }
    }).detach();

    return a.exec();
}
