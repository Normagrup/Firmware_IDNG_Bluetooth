#include "Device.h"
#include "global_variables.h"
#include <QDebug>


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

uint32_t Device::extractUUIDKey(const uint8_t uuid[16], uint8_t deviceID[4])
{
    if (deviceID != nullptr) {
        // Copy the 4-byte device ID into the mapping
        for (int i = 0; i < 4; i++) {
            deviceID[i] = uuid[i];
        }
    } else {
        // If no device ID is available, map to 0xFFFFFFFF
        return 0xFFFFFFFF;
    }
    // Return the first 4 bytes of the UUID as a uint32_t key
    return (uuid[0] << 24) | (uuid[1] << 16) | (uuid[2] << 8) | uuid[3];
}

void Device::configureDevice(uint8_t subnet, uint8_t id, Device* sourceDevice, uint8_t uuid[16], uint8_t manufacturingID[4])
{
    if (subnet < MAX_SUBNET && id < MAX_NODES_SUBNET) {

        // First, clone all configuration data from sourceDevice:
        meshDevice[subnet][id].setIsConfigured(sourceDevice->getIsConfigured());
        meshDevice[subnet][id].setRealAddress(sourceDevice->getRealAddress());
        meshDevice[subnet][id].setDeviceType(sourceDevice->getDeviceType());
        meshDevice[subnet][id].setActualLvl(sourceDevice->getActualLvl());
        meshDevice[subnet][id].setEmergencyMode(sourceDevice->getEmergencyMode());
        meshDevice[subnet][id].setEmergencyFailureStatus(sourceDevice->getEmergencyFailureStatus());

        // Now, build the final UUID:
        uint8_t finalUUID[16];
        memcpy(finalUUID, uuid, 16);  // Copy the original 16-byte UUID.
        // Replace the first 4 bytes with the manufacturing ID if provided, else fill with 0xFF.
        if (manufacturingID != nullptr) {
            for (int i = 0; i < 4; i++) {
                finalUUID[i] = manufacturingID[i];
            }
        } else {
            for (int i = 0; i < 4; i++) {
                finalUUID[i] = 0xFF;
            }
        }
        // Set the final UUID into the device.
        meshDevice[subnet][id].setSerialNumber(finalUUID);
        // Mark device as configured
        meshDevice[subnet][id].setIsConfigured(true);
        // Update the mapping
        uint8_t tempID[4] = {0};
        uint32_t key = extractUUIDKey(finalUUID, tempID);
        uuidToDeviceMap[key] = &meshDevice[subnet][id];
    }
}

bool Device::isDeviceConfigured(uint8_t subnet, uint8_t id)
{
    return (subnet < MAX_SUBNET && id < MAX_NODES_SUBNET) ?
               meshDevice[subnet][id].getIsConfigured() : false;
}

std::vector<Device*> Device::getConfiguredDevices()
{
    std::vector<Device*> configuredDevices;
    for (uint8_t subnet = 0; subnet < MAX_SUBNET; subnet++) {
        for (uint8_t id = 0; id < MAX_NODES_SUBNET; id++) {
            // Check that Nodes[subnet][id] is not null before calling getIsConfigured()
            if (meshDevice[subnet][id].getIsConfigured()) {
                configuredDevices.push_back(&meshDevice[subnet][id]);
            }
        }
    }
    return configuredDevices;
}

void Device::dummyDeviceSet()
{
    // Seed random number generator
    srand(static_cast<unsigned int>(time(0)));
    const int totalDevices = MAX_SUBNET * MAX_NODES_SUBNET;  // 2048

    // Loop over each device slot and configure a device with random values
    for (int i = 0; i < totalDevices; i++) {
        uint8_t subnet = i / MAX_NODES_SUBNET;  // 0 to 31
        uint8_t id     = i % MAX_NODES_SUBNET;   // 0 to 63

        // Generate a random 16-byte UUID (the original random UUID)
        uint8_t randUUID[16];
        for (int j = 0; j < 16; j++) {
            randUUID[j] = rand() % 256;
        }

        // Generate a random 4-byte manufacturing ID
        uint8_t randManufacturing[4];
        for (int j = 0; j < 4; j++) {
            randManufacturing[j] = rand() % 256;
        }

        Device sourceDevice;
        sourceDevice.setIsConfigured(true);
        sourceDevice.setSerialNumber(randUUID);
        sourceDevice.setRealAddress(1234);
        sourceDevice.setDeviceType(2);
        sourceDevice.setActualLvl(50);
        sourceDevice.setEmergencyMode(1);
        sourceDevice.setEmergencyFailureStatus(0);

        // Configure the device's UUID using setDeviceUUID:
        meshDevice[subnet][id].configureDevice(subnet, id, &sourceDevice, randUUID, randManufacturing);
    }
    Device dummy;
    auto configuredList = dummy.getConfiguredDevices();
    qDebug() << "Total configured devices:" << configuredList.size(); // Should be 2048

}

void Device::deleteDevice()
{
    this->_isConfigured = false;
    memset(_serialNumber, 0, 16);
    this->_realAddress = 0;
    memset(_groupSubAddress, 0, sizeof(_groupSubAddress));
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
