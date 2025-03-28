#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QString>

typedef struct {
    QString networkIP;
    QString networkSubmask;
    QString gatewayAddress;
    QString buildingName;
    QString lineName;
    QString password;
    QString mantenedorPassword;
} WebServerData;

typedef struct {
    uint8_t UUID[16];
    uint16_t nodeAddressReport;
} ScannedUUID;

typedef struct {
    bool pollingInProgress;
    bool pollingReceived;
    uint8_t retries;
} PollingData;

typedef struct {
    bool configSecondGroup;
    bool endOfNodeConfiguration;
} GroupDataConfiguration;

typedef struct {
    uint8_t numberOfNodesScanned;
    uint8_t numberOfNodesAdded;
    bool isRelayNode;
    bool isChangeRelayConfirmed;
} CommissionData;

#endif // STRUCTURES_H
