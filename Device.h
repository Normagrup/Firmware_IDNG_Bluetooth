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
    void setUUID(uint8_t* UUID);
    void setRealAddress(uint16_t realAddress);
    void setGroupSubAddress(uint16_t groupAddress);
    bool isOnGroupSubAddress(uint16_t groupAddress);
    bool delGroupSubAddress(uint16_t groupAddress);
    uint16_t* delAllGroups();
    void setCommunicationFailure(bool comunicationFailure);
    void setDeviceType(uint8_t deviceType);
    void setControlGearStatus(uint8_t controlGearStatus);
    void setActualLvl(uint8_t actualLvl);
    void setEmergencyMode(uint8_t emergencyMode);
    void setEmergencyFailureStatus(uint8_t emergencyFailureStatus);

    bool getIsConfigured(void);
    uint8_t* getUUID(void); // devuelve el UUID (16 bytes)
    uint8_t* serialNumber(void); // devuelve el serialNumber (4 últimos bytes del UUID);
    QString serialNumberString(void); // devuelve el serialNumber como un String ("XX.XX.XX.XX")
    uint16_t getRealAddress(void);
    uint16_t getGroupSubAddress(uint8_t position);
    bool getComunicationFailure(void);
    uint8_t getDeviceType(void);
    uint8_t getControlGearStatus(void);
    uint8_t getActualLvl(void);
    uint8_t getEmergencyMode(void);
    uint8_t getEmergencyFailureStatus(void);

    bool hasLampFailure(void);
    bool hasCommunicationFailure(void);
    bool hasBatteryFailure(void);
    bool hasBatteryDurationFailure(void);
    int getTotalFailures(void);
    bool isEmergencyModeActive(void);
    void resetCommunicationFailure();
    void registerCommunicationFailureCycle();

    void deleteDevice(void);

    bool isOnSubList(uint16_t groupAddress);
    bool getPrevLampFail() const { return _prevLampFail; }
    bool getPrevCommFail() const { return _prevCommFail; }
    bool getPrevBatteryFail() const { return _prevBatteryFail; }
    bool getPrevDurationFail() const { return _prevDurationFail; }

    void setPrevLampFail(bool value) { _prevLampFail = value; }
    void setPrevCommFail(bool value) { _prevCommFail = value; }
    void setPrevBatteryFail(bool value) { _prevBatteryFail = value; }
    void setPrevDurationFail(bool value) { _prevDurationFail = value; }

    void copyFrom(const Device& other);

private:
    bool _isConfigured;
    uint8_t _UUID[16];
    uint16_t _realAddress;
    uint16_t _groupSubAddress[MESH_GROUP_COUNT];
    bool _comunicationFailure = false;
    uint8_t _failureCycles = 0;
    uint8_t _deviceType;
    uint8_t _controlGearStatus;
    uint8_t _actualLvl;
    uint8_t _emergencyMode;
    uint8_t _emergencyFailureStatus;
    bool _prevLampFail = false;
    bool _prevCommFail = false;
    bool _prevBatteryFail = false;
    bool _prevDurationFail = false;

};

#endif // DEVICE_H
