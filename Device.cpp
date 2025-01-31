#include "Device.h"

Device::Device(QObject *parent)
    : QObject{parent}
{
    this->_isConfigured = false;
    for (uint8_t i = 0; i < 16; i++) { this->_serialNumber[i] = 0; }
    this->_realAddress = 0;
    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++) { this->_groupSubAddress[i] = 0; }
    this->_comunicationFailure = false;
    this->_deviceType = 0;
    this->_ratedDuration = 0;
    this->_controlGearStatus = 0;
    this->_emergencyFeatures = 0;
    this->_physicalMinLvl = 0;
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

void Device::setSerialNumber(uint8_t* serialNumber)
{
    for (uint8_t i = 0; i < 16; i++) { _serialNumber[i] = serialNumber[i];}
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

void Device::delGroupSubAddress(uint16_t groupAddress)
{
    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++) {
        if (_groupSubAddress[i] == groupAddress) {
            _groupSubAddress[i] = 0;
            break;
        }
    }
}

void Device::setCommunicationFailure(bool comunicationFailure)
{
    _comunicationFailure = comunicationFailure;
}

void Device::setDeviceType(uint8_t deviceType)
{
    _deviceType = deviceType;
}

void Device::setRatedDuration(uint8_t ratedDuration)
{
    _ratedDuration = ratedDuration;
}

void Device::setControlGearStatus(uint8_t controlGearStatus)
{
    _controlGearStatus = controlGearStatus;
}

void Device::setEmergencyFeatures(uint8_t emergencyFeatures)
{
    _emergencyFeatures = emergencyFeatures;
}

void Device::setPhysicalMinLvl(uint8_t physicalMinLvl)
{
    _physicalMinLvl = physicalMinLvl;
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

uint8_t* Device::getSerialNumber()
{
    return _serialNumber;
}

uint16_t Device::getRealAddress()
{
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

uint8_t Device::getRatedDuration()
{
    return _ratedDuration;
}

uint8_t Device::getControlGearStatus()
{
    return _controlGearStatus;
}

uint8_t Device::getEmergencyFeatures()
{
    return _emergencyFeatures;
}

uint8_t Device::getPhysicalMinLvl()
{
    return _physicalMinLvl;
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

void Device::deleteDevice()
{
    this->_isConfigured = false;
    for (uint8_t i = 0; i < 16; i++) { this->_serialNumber[i] = 0; }
    this->_realAddress = 0;
    for (uint8_t i = 0; i < MESH_GROUP_COUNT; i++) { this->_groupSubAddress[i] = 0; }
    this->_comunicationFailure = false;
    this->_deviceType = 0;
    this->_ratedDuration = 0;
    this->_controlGearStatus = 0;
    this->_emergencyFeatures = 0;
    this->_physicalMinLvl = 0;
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
