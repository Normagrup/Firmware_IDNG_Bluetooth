#include "Pollings.h"
#include "global_variables.h"

Pollings::Pollings(QObject *parent)
    : QObject{parent}
{
    setConfiguredSubnets();
}

void Pollings::setConfiguredSubnets()
{
    _configuredSubnets = 0;
    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (meshDevice[i][j].getIsConfigured()) {
                _configuredSubnets++;
                break;
            }
        }
    }
}

uint8_t Pollings::getConfiguredSubnets()
{
    return _configuredSubnets;
}

bool Pollings::isSubnetConfigured(uint8_t subnet)
{
    if (subnet < getConfiguredSubnets()) { return true; }
    else { return false; }
}

QByteArray Pollings::getArrayConfiguredDevices(uint8_t subnet)
{
    QByteArray frame;
    frame.append(0xFF);
    frame.append(0xFF);
    frame.append(0xFF);
    frame.append(0xFF);
    frame.append(0xFF);
    frame.append(0xFF);
    frame.append(0xFF);
    frame.append(0xFF);

    for (uint8_t i = (MAX_NODES_SUBNET - 1); i >= 0; i--){
        if (meshDevice[subnet][i].getIsConfigured()) { frame.append(0x01); }
        else { frame.append((unsigned char)0x00); }
    }

    return frame;
}

QByteArray Pollings::getArrayDevicesType(uint8_t subnet)
{
    QByteArray frame;
    frame.append(this->getByteDeviceTypeConfigured(subnet, 7));
    frame.append(this->getByteDeviceTypeConfigured(subnet, 6));
    frame.append(this->getByteDeviceTypeConfigured(subnet, 5));
    frame.append(this->getByteDeviceTypeConfigured(subnet, 4));
    frame.append(this->getByteDeviceTypeConfigured(subnet, 3));
    frame.append(this->getByteDeviceTypeConfigured(subnet, 2));
    frame.append(this->getByteDeviceTypeConfigured(subnet, 1));
    frame.append(this->getByteDeviceTypeConfigured(subnet, 0));

    for (uint8_t i = (MAX_NODES_SUBNET - 1); i >= 0; i--){
        if (meshDevice[subnet][i].getIsConfigured()) { frame.append(0x01); } // EMERGENCY TYPE 0x01
        else { frame.append((unsigned char)0x00); }
    }

    return frame;
}

QByteArray Pollings::getArrayComunicationFailure(uint8_t subnet)
{
    QByteArray frame;
    frame.append(this->getByteComunicationFailureConfigured(subnet, 7));
    frame.append(this->getByteComunicationFailureConfigured(subnet, 6));
    frame.append(this->getByteComunicationFailureConfigured(subnet, 5));
    frame.append(this->getByteComunicationFailureConfigured(subnet, 4));
    frame.append(this->getByteComunicationFailureConfigured(subnet, 3));
    frame.append(this->getByteComunicationFailureConfigured(subnet, 2));
    frame.append(this->getByteComunicationFailureConfigured(subnet, 1));
    frame.append(this->getByteComunicationFailureConfigured(subnet, 0));

    for (uint8_t i = (MAX_NODES_SUBNET - 1); i >= 0; i--){
        if (meshDevice[subnet][i].getIsConfigured() && meshDevice[subnet][i].getComunicationFailure()) { frame.append(0x01); }
        else { frame.append((unsigned char)0x00); }
    }

    return frame;
}

QByteArray Pollings::getArrayQueryActualLvl(uint8_t subnet)
{
    QByteArray frame;
    frame.append(this->getByteActualLvlConfigured(subnet, 7));
    frame.append(this->getByteActualLvlConfigured(subnet, 6));
    frame.append(this->getByteActualLvlConfigured(subnet, 5));
    frame.append(this->getByteActualLvlConfigured(subnet, 4));
    frame.append(this->getByteActualLvlConfigured(subnet, 3));
    frame.append(this->getByteActualLvlConfigured(subnet, 2));
    frame.append(this->getByteActualLvlConfigured(subnet, 1));
    frame.append(this->getByteActualLvlConfigured(subnet, 0));

    for (uint8_t i = (MAX_NODES_SUBNET - 1); i >= 0; i--){
        if (meshDevice[subnet][i].getIsConfigured()) {
            if (!meshDevice[subnet][i].getComunicationFailure()) { frame.append(meshDevice[subnet][i].getActualLvl()); }
            else { frame.append(0x80); }
        }
        else { frame.append((unsigned char)0x00); }
    }

    return frame;
}

QByteArray Pollings::getArrayQueryStatus(uint8_t subnet)
{
    QByteArray frame;
    frame.append(this->getByteStatusConfigured(subnet, 7));
    frame.append(this->getByteStatusConfigured(subnet, 6));
    frame.append(this->getByteStatusConfigured(subnet, 5));
    frame.append(this->getByteStatusConfigured(subnet, 4));
    frame.append(this->getByteStatusConfigured(subnet, 3));
    frame.append(this->getByteStatusConfigured(subnet, 2));
    frame.append(this->getByteStatusConfigured(subnet, 1));
    frame.append(this->getByteStatusConfigured(subnet, 0));

    for (uint8_t i = (MAX_NODES_SUBNET - 1); i >= 0; i--){
        if (meshDevice[subnet][i].getIsConfigured()) {
            if (meshDevice[subnet][i].getComunicationFailure()) { frame.append(0x80); }
            else { frame.append(0x40); }
        }
        else { frame.append((unsigned char)0x00); }
    }

    return frame;
}

QByteArray Pollings::getArrayQueryEmergencyMode(uint8_t subnet)
{
    QByteArray frame;
    frame.append(this->getByteEmergencyModeConfigured(subnet, 7));
    frame.append(this->getByteEmergencyModeConfigured(subnet, 6));
    frame.append(this->getByteEmergencyModeConfigured(subnet, 5));
    frame.append(this->getByteEmergencyModeConfigured(subnet, 4));
    frame.append(this->getByteEmergencyModeConfigured(subnet, 3));
    frame.append(this->getByteEmergencyModeConfigured(subnet, 2));
    frame.append(this->getByteEmergencyModeConfigured(subnet, 1));
    frame.append(this->getByteEmergencyModeConfigured(subnet, 0));

    for (uint8_t i = (MAX_NODES_SUBNET - 1); i >= 0; i--){
        if (meshDevice[subnet][i].getIsConfigured()) {
            if (!meshDevice[subnet][i].getComunicationFailure()) { frame.append(meshDevice[subnet][i].getEmergencyMode()); }
            else { frame.append(0x80); }
        }
        else { frame.append((unsigned char)0x00); }
    }

    return frame;
}

QByteArray Pollings::getArrayQueryFailureStatus(uint8_t subnet)
{
    QByteArray frame;
    frame.append(this->getByteFailureStatusConfigured(subnet, 7));
    frame.append(this->getByteFailureStatusConfigured(subnet, 6));
    frame.append(this->getByteFailureStatusConfigured(subnet, 5));
    frame.append(this->getByteFailureStatusConfigured(subnet, 4));
    frame.append(this->getByteFailureStatusConfigured(subnet, 3));
    frame.append(this->getByteFailureStatusConfigured(subnet, 2));
    frame.append(this->getByteFailureStatusConfigured(subnet, 1));
    frame.append(this->getByteFailureStatusConfigured(subnet, 0));

    for (uint8_t i = (MAX_NODES_SUBNET - 1); i >= 0; i--){
        if (meshDevice[subnet][i].getIsConfigured()) {
            if (!meshDevice[subnet][i].getComunicationFailure()) { frame.append(meshDevice[subnet][i].getEmergencyFailureStatus()); }
            else { frame.append(0x80); }
        }
        else { frame.append((unsigned char)0x00); }
    }

    return frame;
}

uint8_t Pollings::getByteDeviceTypeConfigured(uint8_t subnet, uint8_t netPosition)
{
    uint8_t deviceAddress, byteDeviceTypeConfigured = 0;
    for (uint8_t i = 0; i < 8; i++) {
        deviceAddress = (netPosition * 8) + i;
        if (meshDevice[subnet][deviceAddress].getIsConfigured()) { byteDeviceTypeConfigured += (0x01 << i);  }
    }
    return byteDeviceTypeConfigured;
}

uint8_t Pollings::getByteComunicationFailureConfigured(uint8_t subnet, uint8_t netPosition)
{
    uint8_t deviceAddress, byteComunicationFailureConfigured = 0;
    for (uint8_t i = 0; i < 8; i++) {
        deviceAddress = (netPosition * 8) + i;
        if (meshDevice[subnet][deviceAddress].getIsConfigured()) { byteComunicationFailureConfigured += (0x01 << i);  }
    }
    return byteComunicationFailureConfigured;
}

uint8_t Pollings::getByteActualLvlConfigured(uint8_t subnet, uint8_t netPosition)
{
    uint8_t deviceAddress, byteActualLvlConfigured = 0;
    for (uint8_t i = 0; i < 8; i++) {
        deviceAddress = (netPosition * 8) + i;
        if (meshDevice[subnet][deviceAddress].getIsConfigured() && !meshDevice[subnet][i].getComunicationFailure()) { byteActualLvlConfigured += (0x01 << i);  }
    }
    return byteActualLvlConfigured;
}

uint8_t Pollings::getByteStatusConfigured(uint8_t subnet, uint8_t netPosition)
{
    return (unsigned char)0x00;
}

uint8_t Pollings::getByteEmergencyModeConfigured(uint8_t subnet, uint8_t netPosition)
{
    uint8_t deviceAddress, byteEmergencyModeConfigured = 0;
    for (uint8_t i = 0; i < 8; i++) {
        deviceAddress = (netPosition * 8) + i;
        if (meshDevice[subnet][deviceAddress].getIsConfigured() && !meshDevice[subnet][i].getComunicationFailure()) { byteEmergencyModeConfigured += (0x01 << i);  }
    }
    return byteEmergencyModeConfigured;
}

uint8_t Pollings::getByteFailureStatusConfigured(uint8_t subnet, uint8_t netPosition)
{
    uint8_t deviceAddress, byteFailureStatusConfigured = 0;
    for (uint8_t i = 0; i < 8; i++) {
        deviceAddress = (netPosition * 8) + i;
        if (meshDevice[subnet][deviceAddress].getIsConfigured() && !meshDevice[subnet][i].getComunicationFailure()) { byteFailureStatusConfigured += (0x01 << i);  }
    }
    return byteFailureStatusConfigured;
}
