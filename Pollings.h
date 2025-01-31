#ifndef POLLINGS_H
#define POLLINGS_H

#include <QObject>

#define SUBNET_OFFSET           0

class Pollings : public QObject
{
    Q_OBJECT
public:
    explicit Pollings(QObject *parent = nullptr);

    void setConfiguredSubnets(void);
    uint8_t getConfiguredSubnets(void);
    bool isSubnetConfigured(uint8_t subnet);

    QByteArray getArrayConfiguredDevices(uint8_t subnet);
    QByteArray getArrayDevicesType(uint8_t subnet);
    QByteArray getArrayComunicationFailure(uint8_t subnet);
    QByteArray getArrayQueryActualLvl(uint8_t subnet);
    QByteArray getArrayQueryStatus(uint8_t subnet);
    QByteArray getArrayQueryEmergencyMode(uint8_t subnet);
    QByteArray getArrayQueryFailureStatus(uint8_t subnet);

    uint8_t getByteDeviceTypeConfigured(uint8_t subnet, uint8_t netPosition);
    uint8_t getByteComunicationFailureConfigured(uint8_t subnet, uint8_t netPosition);
    uint8_t getByteActualLvlConfigured(uint8_t subnet, uint8_t netPosition);
    uint8_t getByteStatusConfigured(uint8_t subnet, uint8_t netPosition);
    uint8_t getByteEmergencyModeConfigured(uint8_t subnet, uint8_t netPosition);
    uint8_t getByteFailureStatusConfigured(uint8_t subnet, uint8_t netPosition);

signals:

private:
    uint8_t _configuredSubnets = 0;
};

#endif // POLLINGS_H
