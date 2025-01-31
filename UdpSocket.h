#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>
#include <QQueue>

#define UDP_PORT_NUMBER         2234

class UdpSocket : public QObject
{
    Q_OBJECT
public:
    explicit UdpSocket(QObject *parent = nullptr);
    ~UdpSocket();

    bool sendData(QHostAddress dstAddress, QByteArray data);

signals:
    bool dataReceived(QQueue <QPair <QString, QByteArray> >* rcvData);

private slots:
    void dataReady(void);

private:
    QUdpSocket* _udpSocket;

    QQueue <QPair <QString, QByteArray> > rcvData;
};

#endif // UDPSOCKET_H


