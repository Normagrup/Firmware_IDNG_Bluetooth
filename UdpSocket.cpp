#include "UdpSocket.h"

UdpSocket::UdpSocket(QObject *parent)
    : QObject{parent}
{
    _udpSocket = new QUdpSocket(this);
    _udpSocket->bind(UDP_PORT_NUMBER);
    connect(_udpSocket, &QUdpSocket::readyRead, this, &UdpSocket::dataReady);
}

UdpSocket::~UdpSocket()
{
    if (_udpSocket != nullptr) {
        _udpSocket->close();
        delete _udpSocket;
    }
}

bool UdpSocket::sendData(QHostAddress dstAddress, QByteArray data)
{
    if (_udpSocket == nullptr) { return false; }
    _udpSocket->writeDatagram(data, dstAddress, UDP_PORT_NUMBER);
    _udpSocket->waitForBytesWritten(10);
    return true;
}

void UdpSocket::dataReady()
{
    while (_udpSocket->hasPendingDatagrams()) {
        QByteArray dataBuffer;
        QHostAddress rcvAddress;
        uint16_t rcvPort;

        dataBuffer.resize(_udpSocket->pendingDatagramSize());
        _udpSocket->readDatagram(dataBuffer.data(), dataBuffer.size(), &rcvAddress, &rcvPort);

        rcvData.enqueue(QPair <QString, QByteArray> (rcvAddress.toString(), dataBuffer));
        emit dataReceived(&rcvData);
    }
}
