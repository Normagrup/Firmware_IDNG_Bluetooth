#ifndef WIRELESS_H
#define WIRELESS_H

#include <QObject>
#include <QTimer>

#include "UartPort.h"
#include "UdpSocket.h"
#include "Device.h"
#include "WebServer.h"
#include "Database.h"
class EmbeddedIO;
class Wireless : public QObject
{
    Q_OBJECT
public:
    explicit Wireless(QObject *parent = nullptr);

    void runNetwork(void);
    void udpReceivedData(QQueue <QPair <QString, QByteArray> >* rcvData);
    void uartReceivedData(QByteArray data);
    void webServerReceivedData(QString data);
    void setEmbeddedIO(EmbeddedIO* io);

private slots:
    void pollingTimerHandler(void);
    void testTimerHandler(void);
    void checkTestResultsHandler(void);
    void groupFrameTimerHandler(void);
    void addDeviceTimerHandler(void);
    void confirmAddDeviceTimerHandler(void);
    void newIterationTimerHandler(void);
    void replaceP2TimerHandler(void);
    void replaceP3TimerHandler(void);
    void cleanCdbTimerHandler(void);
    void askInitDataFromMicroTimerHandler(void);

    void updateLogsByPollings(Device &device);
    void updateLogsByTests(uint8_t i, uint8_t code);
    void onTestButtonPressed();

signals:

private:
    UartPort* _uartPort;
    UdpSocket* _udpSocket;
    WebServer* _webServer;
    Database* _database;
    EmbeddedIO* _io;
};

#endif // WIRELESS_H
