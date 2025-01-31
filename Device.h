#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>

#define MESH_GROUP_COUNT        32

class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(QObject *parent = nullptr);
    ~Device();

    void setIsConfigured(bool isConfigured);
    void setSerialNumber(uint8_t* serialNumber);
    void setRealAddress(uint16_t realAddress);
    void setGroupSubAddress(uint16_t groupAddress);
    void delGroupSubAddress(uint16_t groupAddress);
    void setCommunicationFailure(bool comunicationFailure);
    void setDeviceType(uint8_t deviceType);
    void setRatedDuration(uint8_t ratedDuration);
    void setControlGearStatus(uint8_t controlGearStatus);
    void setEmergencyFeatures(uint8_t emergencyFeatures);
    void setPhysicalMinLvl(uint8_t physicalMinLvl);
    void setActualLvl(uint8_t actualLvl);
    void setEmergencyMode(uint8_t emergencyMode);
    void setEmergencyFailureStatus(uint8_t emergencyFailureStatus);

    bool getIsConfigured(void);
    uint8_t* getSerialNumber(void);
    uint16_t getRealAddress(void);
    uint16_t getGroupSubAddress(uint8_t position);
    bool getComunicationFailure(void);
    uint8_t getDeviceType(void);
    uint8_t getRatedDuration(void);
    uint8_t getControlGearStatus(void);
    uint8_t getEmergencyFeatures(void);
    uint8_t getPhysicalMinLvl(void);
    uint8_t getActualLvl(void);
    uint8_t getEmergencyMode(void);
    uint8_t getEmergencyFailureStatus(void);

    void deleteDevice(void);

    bool isOnSubList(uint16_t groupAddress);

private:
    bool _isConfigured;
    uint8_t _serialNumber[16];
    uint16_t _realAddress;
    uint16_t _groupSubAddress[MESH_GROUP_COUNT];
    bool _comunicationFailure;
    uint8_t _deviceType;
    uint8_t _ratedDuration;
    uint8_t _controlGearStatus;
    uint8_t _emergencyFeatures;
    uint8_t _physicalMinLvl;
    uint8_t _actualLvl;
    uint8_t _emergencyMode;
    uint8_t _emergencyFailureStatus;
};

#endif // DEVICE_H
