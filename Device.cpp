#include "Device.h"
#include "global_variables.h"

Device::Device(QObject *parent)
    : QObject{parent}
{
    this->_isConfigured = false;
    for (uint8_t i = 0; i < 16; i++) { this->_UUID[i] = 0; }
    this->_realAddress = 0;
    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++) { this->_groupSubAddress[i] = 0; }
    this->_comunicationFailure = false;
    this->_deviceType = 0;
    this->_controlGearStatus = 0;
    this->_actualLvl = 0;
    this->_emergencyMode = 0;
    this->_emergencyFailureStatus = 0;
}

Device::~Device()
{

}

void Device::setIsConfigured(bool isConfigured)
{
    _isConfigured = isConfigured;
}

void Device::setUUID(uint8_t* UUID)
{
    for (uint8_t i = 0; i < 16; i++) { _UUID[i] = UUID[i];}
}

void Device::setRealAddress(uint16_t realAddress)
{
    _realAddress = realAddress;
}

void Device::setGroupSubAddress(uint16_t groupAddress)
{
    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++) {
        if (_groupSubAddress[i] == groupAddress) { return; }
    }

    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++) {
        if (_groupSubAddress[i] == 0) {
            _groupSubAddress[i] = groupAddress;
            break;
        }
    }
}

bool Device::isOnGroupSubAddress(uint16_t groupAddress)
{
    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++)
        if (_groupSubAddress[i] == groupAddress)
            return true;
    return false;
}

bool Device::delGroupSubAddress(uint16_t groupAddress)
{
    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++) {
        if (_groupSubAddress[i] == groupAddress) {
            _groupSubAddress[i] = 0;
            return true; // el device pertenecía al grupo y se le elimina
        }
    }
    return false; // el device no pertenecía al grupo
}

uint16_t* Device::delAllGroups()
{
    static uint16_t removedGroups[MESH_GROUP_COUNT];
    uint8_t count = 0;

    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++) {
        if (_groupSubAddress[i] != 0) {
            removedGroups[count++] = _groupSubAddress[i];
            _groupSubAddress[i] = 0;
        }
    }

    for (uint8_t j = count; j < MESH_GROUP_COUNT; j++) {
        removedGroups[j] = 0;  // rellenar el resto con ceros
    }

    return removedGroups;
}

void Device::setCommunicationFailure(bool comunicationFailure)
{
    _comunicationFailure = comunicationFailure;
}

void Device::setDeviceType(uint8_t deviceType)
{
    _deviceType = deviceType;
}

void Device::setControlGearStatus(uint8_t controlGearStatus)
{
    _controlGearStatus = controlGearStatus;
}

void Device::setActualLvl(uint8_t actualLvl)
{
    _actualLvl = actualLvl;
}

void Device::setEmergencyMode(uint8_t emergencyMode)
{
    _emergencyMode = emergencyMode;
}

void Device::setEmergencyFailureStatus(uint8_t emergencyFailureStatus)
{
    _emergencyFailureStatus = emergencyFailureStatus;
}

bool Device::getIsConfigured()
{
    return _isConfigured;
}

uint8_t* Device::getUUID()
{
    return _UUID;
}

uint8_t* Device::serialNumber()
{
    uint8_t* UUID = getUUID();
    uint8_t* SN = new uint8_t[4];

    for(int i = 0; i <= 3; i++)
        SN[i] = UUID[i + 12];

    return SN;
}

QString Device::serialNumberString()
{
    uint8_t* SN = serialNumber();
    return QString("%1.%2.%3.%4")
        .arg(SN[0], 2, 16, QLatin1Char('0')).toUpper()
        .arg(SN[1], 2, 16, QLatin1Char('0')).toUpper()
        .arg(SN[2], 2, 16, QLatin1Char('0')).toUpper()
        .arg(SN[3], 2, 16, QLatin1Char('0')).toUpper();
}

uint16_t Device::getRealAddress()
{   
    //printf(" Obteniendo dirección real del nodo: %04X\n", this->_realAddress);
    return _realAddress;
}

uint16_t Device::getGroupSubAddress(uint8_t position)
{
    return _groupSubAddress[position];
}

bool Device::getComunicationFailure()
{
    return _comunicationFailure;
}

uint8_t Device::getDeviceType()
{
    return _deviceType;
}

uint8_t Device::getControlGearStatus()
{
    return _controlGearStatus;
}

uint8_t Device::getActualLvl()
{
    return _actualLvl;
}

uint8_t Device::getEmergencyMode()
{
    return _emergencyMode;
}

uint8_t Device::getEmergencyFailureStatus()
{
    return _emergencyFailureStatus;
}

bool Device::hasLampFailure()
{
    uint8_t controlGearStatus = getControlGearStatus();
    int lampFailureBit = 1;
    return ((controlGearStatus >> lampFailureBit) & 1);
}

bool Device::hasCommunicationFailure()
{
    return getComunicationFailure();
}

bool Device::hasBatteryFailure()
{
    uint8_t emergencyFailureStatus = getEmergencyFailureStatus();
    int batteryFailureBit = 2;
    return ((emergencyFailureStatus >> batteryFailureBit) & 1);
}

bool Device::hasBatteryDurationFailure()
{
    uint8_t emergencyFailureStatus = getEmergencyFailureStatus();
    int durationFailureBit = 1;
    return ((emergencyFailureStatus >> durationFailureBit) & 1);
}

int Device::getTotalFailures()
{
    int totalFailures = 0;

    // Fallo de lámpara
    if(hasLampFailure()) { totalFailures++; }

    // Fallos de duración y batería
    if(hasBatteryDurationFailure()) { totalFailures++; }
    if(hasBatteryFailure()) { totalFailures++; }

    // Fallo de comunicación
    if(hasCommunicationFailure()) { totalFailures++; }

    return totalFailures;
}

bool Device::isEmergencyModeActive()
{
    uint8_t emergencyMode = getEmergencyMode();
    int emergencyModeActiveBit = 2;
    return ((emergencyMode >> emergencyModeActiveBit) & 1);
}

void Device::deleteDevice()
{
    this->_isConfigured = false;
    for (uint8_t i = 0; i < 16; i++) { this->_UUID[i] = 0; }
    this->_realAddress = 0;
    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++) { this->_groupSubAddress[i] = 0; }
    this->_comunicationFailure = false;
    this->_deviceType = 0;
    this->_controlGearStatus = 0;
    this->_actualLvl = 0;
    this->_emergencyMode = 0;
    this->_emergencyFailureStatus = 0;
}

bool Device::isOnSubList(uint16_t groupAddress)
{
    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++) {
        if (_groupSubAddress[i] == groupAddress) { return true; }
    }
    return false;
}

void Device::resetCommunicationFailure()
{
    _failureCycles = 0;
    setCommunicationFailure(false);
}

void Device::registerCommunicationFailureCycle()
{
    _failureCycles++;
    if (_failureCycles >= failComCycles) {
        setCommunicationFailure(true);
    }
}

void Device::copyFrom(const Device& other)
{
    _isConfigured = other._isConfigured;
    _realAddress = other._realAddress;
    _comunicationFailure = other._comunicationFailure;
    _failureCycles = other._failureCycles;
    _deviceType = other._deviceType;
    _controlGearStatus = other._controlGearStatus;
    _actualLvl = other._actualLvl;
    _emergencyMode = other._emergencyMode;
    _emergencyFailureStatus = other._emergencyFailureStatus;
    _prevLampFail = other._prevLampFail;
    _prevCommFail = other._prevCommFail;
    _prevBatteryFail = other._prevBatteryFail;
    _prevDurationFail = other._prevDurationFail;

    memcpy(_UUID, other._UUID, sizeof(_UUID));
    memcpy(_groupSubAddress, other._groupSubAddress, sizeof(_groupSubAddress));
}
