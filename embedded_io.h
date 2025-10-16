#pragma once
#include <QObject>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QCoreApplication>
#include <unistd.h>

#include "file_handler.h"
#include "Database.h"
#include "eth_frames.h"

#define GPIO_I09_BTN_FACTORY   80
#define GPIO_I06_LINK_LED      82
#define GPIO_I07_HEARTBEAT     81
#ifndef IFACE_NAME
#define IFACE_NAME "eth0"
#endif

constexpr int BOOT_BLINK_MS = 250;     // parpadeo rápido al arrancar
constexpr int READY_BLINK_MS = 0;
constexpr int DEGRADED_BLINK_MS = 1000;
constexpr int FACTORY_HOLD_MS = 2000; // mantener 2 s para reset IP a fábrica
constexpr int FAIL_BLINK_MS = 250;

constexpr int BTN_SAMPLE_MS   = 20;


class EmbeddedIO : public QObject {
    Q_OBJECT
public:


    enum class LedMode { Off, On, Blink };
    explicit EmbeddedIO(QObject* parent=nullptr);
    ~EmbeddedIO();

    void markBooting();
    void markReady();
    void beginRebootSequence();

private:
    static bool exportGpio(int n);
    static bool setDir(int n, const char* dir);
    static bool setEdge(int n, const char* edge);
    static bool writeVal(int n, int v);
    static int  readVal(int n);

    void initGpios();

    void onFactoryButtonSample();

    void doFactoryResetAndReboot();

    void setIPAddressFileV2(QString ipAddress);
    void setSubmaskAddressFileV2(QString submaskAddress);
    void setGatewayAddressFileV2(QString gatewayAddress);
    void setIPConfigInfoV2(QStringList webServerParts, Database* database);

    // estado/motores de los leds
    void setLinkMode(LedMode m, int periodMs = 300);
    void setFailMode(LedMode m, int periodMs = 300);
    void setLinkLed(bool on);
    void setFailLed(bool on);

    LedMode _linkMode = LedMode::Off;
    LedMode _failMode = LedMode::Off;

    QTimer _linkTimer;
    QTimer _btnTimer;
    QTimer _mcuReqTimer;
    QTimer _failTimer;

    qint64 _startMs = 0;
    bool   _factoryArmed = false;
    bool _linkLevel = false;
    bool   _failLevel = false;
    int    _btnAccumMs = 0;
    int    _btnReleaseMs = 0;
    int    _btnIdleLevel = 1;

    int _linkBlinkMs = BOOT_BLINK_MS;
    int _failBlinkMs = FAIL_BLINK_MS;

private slots:
    void onLinkTick();
    void onFailTick();


};
