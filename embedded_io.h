#pragma once
#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <unistd.h>
#include "Database.h"
#include "eth_frames.h"
#include <QCoreApplication>

// ======= CONFIGURA AQUÍ TUS NÚMEROS DE GPIO (Linux) =======
#define GPIO_I09_BTN_FACTORY   80
#define GPIO_I06_LINK_LED      81
#define GPIO_I07_HEARTBEAT     80
#define GPIO_I08_MCU_REQ_RST   79
// ===========================================================

// Frecuencias/tiempos
constexpr int HB_PERIOD_MS = 500;      // ~2 Hz
constexpr int BOOT_BLINK_MS = 250;     // parpadeo rápido al arrancar
constexpr int READY_BLINK_MS = 0;      // 0 = fijo
constexpr int DEGRADED_BLINK_MS = 1000;
constexpr int FACTORY_HOLD_MS = 2000; // mantener 10 s para reset a fábrica
#ifndef IFACE_NAME
#define IFACE_NAME "eth0"
#endif

constexpr int BTN_SAMPLE_MS   = 20;

class EmbeddedIO : public QObject {
    Q_OBJECT
public:
    enum class LinkState { Booting, Ready, Degraded };

    explicit EmbeddedIO(QObject* parent=nullptr);
    ~EmbeddedIO();

    // Llamar tras tener todo listo (DB, red, etc.)
    void markReady() { setLinkState(LinkState::Ready); }

    // O usar esta si quieres indicar estado degradado
    void setDegraded() { setLinkState(LinkState::Degraded); }

signals:
    void factoryResetTriggered();  // por si quieres enganchar lógica adicional

private:
    // ---- GPIO helpers (sysfs) ----
    static bool exportGpio(int n);
    static bool unexportGpio(int n);
    static bool setDir(int n, const char* dir); // "in" / "out"
    static bool setEdge(int n, const char* edge); // "none"/"rising"/"falling"/"both"
    static bool writeVal(int n, int v);
    static int  readVal(int n);

    void initGpios();
    void setLinkState(LinkState s);
    void updateLinkLed();
    void onFactoryButtonSample();
    void onMcuReqRstSample();
    void doFactoryResetAndReboot();
    void softRebootProcess();
    void setIPAddressFileV2(QString ipAddress);
    void setSubmaskAddressFileV2(QString submaskAddress);
    void setGatewayAddressFileV2(QString gatewayAddress);
    void setIPConfigInfoV2(QStringList webServerParts, Database* database);


    QTimer _hbTimer;          // toggle a I07
    QTimer _linkTimer;        // parpadeo LED I06
    QTimer _btnTimer;         // muestreo botón I09
    QTimer _mcuReqTimer;      // muestreo I08 (opcional)
    qint64 _startMs = 0;
    bool   _factoryArmed = false;
    int    _btnAccumMs = 0;
    int    _btnReleaseMs = 0;
    int    _btnIdleLevel = 1;   // se autodetecta en initGpios()


    LinkState _state = LinkState::Booting;
    int _linkBlinkMs = BOOT_BLINK_MS;
    bool _linkLevel = false;
    qint64 _btnPressedSinceMs = 0;
};
