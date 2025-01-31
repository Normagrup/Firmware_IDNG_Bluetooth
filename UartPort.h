#ifndef UARTPORT_H
#define UARTPORT_H

#include <QObject>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>

#define UART_PORT_NAME         "/dev/ttyS1"
#define UART_BAUD_RATE          115200

class UartPort : public QObject
{
    Q_OBJECT
public:
    explicit UartPort(QObject *parent = nullptr);
    ~UartPort();

    bool sendData(QByteArray data);

signals:
    bool dataReceived(QByteArray data);

private slots:
    void dataReady(void);

private:
    QSerialPort* _uartPort;
};

#endif // UARTPORT_H
