#ifndef WIRELESS_H
#define WIRELESS_H

#include <QObject>
#include <QTimer>

#include "UartPort.h"
#include "UdpSocket.h"
#include "Device.h"
#include "WebServer.h"
#include "Database.h"

class Wireless : public QObject
{
    Q_OBJECT
public:
    explicit Wireless(QObject *parent = nullptr);

    void runNetwork(void);
    void udpReceivedData(QQueue <QPair <QString, QByteArray> >* rcvData);
    void uartReceivedData(QByteArray data);
    void webServerReceivedData(QString data);

private slots:
    void pollingTimerHandler(void);
    void testTimerHandler(void);
    void groupFrameTimerHandler(void);
    void addDeviceTimerHandler(void);
    void confirmAddDeviceTimerHandler(void);
    void newIterationTimerHandler(void);

signals:

private:
    UartPort* _uartPort;
    UdpSocket* _udpSocket;
    WebServer* _webServer;
    Database* _database;
};

#endif // WIRELESS_H
