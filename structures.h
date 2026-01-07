#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QString>
#include <QDateTime>
#include "UdpSocket.h"

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

typedef struct {
    QString name;
    QString serialNum;
    int btAddress;
    QString devIP;
    qint64 timestamp;
    int event;
    QString eventType;
} LogInfo;

typedef struct {
    uint16_t groupId;
    QString testType; // "FUNCTIONAL" or "DURATION"
    QTime checkTime;
} AntennaTestCheck;

typedef struct {
    QDateTime timestamp;
    QString ip;
} AntennaInfo;

typedef struct {
    uint8_t subnetAddress;
    uint8_t nodeSubnetAddress;
    uint16_t realAddress;
    QString serialNumber;
} NodeInfo;

typedef struct {
    uint8_t groupId;
    uint8_t subnetId;
    QByteArray bitmap;
} GroupBitmap;

typedef struct {
    uint16_t pid;
    QString rcvAddress;
    UdpSocket* socket;
} POLQueryContext;

typedef struct {
    uint16_t groupId;
    uint8_t subnetId;
    QByteArray bitmap;
} writeGroupBitmap;

typedef enum {
    Add,
    Remove
} GroupActionType;

typedef struct {
    QString newNodeUUID;
    uint16_t newNodeRealAddress; // lazy
    QString oldNodeID;
    uint16_t oldNodeRealAddress; // lazy
} ReplaceData;

typedef struct {
    uint8_t subnetAddress;
    uint8_t nodeSubnetAddress;
    QString groupSubAddress;
    uint8_t relayMode;
} ReplaceNode;

typedef struct {
    QString serial;
    uint16_t netAddress;
    uint16_t bluetoothAddress;
    QString installKey;
} UnassignedNode;

typedef struct {
    int i;
    int j;
    QString serial;
} CommissionedNode;

#endif // STRUCTURES_H
