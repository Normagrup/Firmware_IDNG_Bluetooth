#include "UartPort.h"

UartPort::UartPort(QObject *parent)
    : QObject{parent}
{
    _uartPort = new QSerialPort(this);
    _uartPort->setPortName(UART_PORT_NAME);
    _uartPort->setBaudRate(UART_BAUD_RATE);
    _uartPort->setDataBits(QSerialPort::Data8);
    _uartPort->setParity(QSerialPort::NoParity);
    _uartPort->setStopBits(QSerialPort::OneStop);

    if (_uartPort->open(QIODevice::ReadWrite)) { connect(_uartPort, &QSerialPort::readyRead, this, &UartPort::dataReady); }
}

UartPort::~UartPort()
{
    if (_uartPort != nullptr) {
        _uartPort->close();
        delete _uartPort;
    }
}

bool UartPort::sendData(QByteArray data)
{
    if (_uartPort == nullptr || !_uartPort->isOpen()) { return false; }
    _uartPort->write(data);
    _uartPort->waitForBytesWritten(5);
    return true;
}

void UartPort::dataReady()
{
    if (_uartPort->isOpen()) { emit dataReceived(_uartPort->readAll()); }
}
