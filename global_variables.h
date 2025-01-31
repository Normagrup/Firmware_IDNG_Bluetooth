#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include <QTimer>

#include "structures.h"
#include "Device.h"
#include "Pollings.h"
#include "Tests.h"

#define MAX_SUBNET              32
#define MAX_NODES_SUBNET        64
#define MAX_TEST                17

extern WebServerData webServerData;
extern QString networkIP[4];
extern QString networkSubmask[4];
extern QString gatewayAddress[4];
extern QString macAddress[6];

extern Device meshDevice[MAX_SUBNET][MAX_NODES_SUBNET];
//extern ScannedUUID scannedUUID[MAX_SUBNET * MAX_NODES_SUBNET];
extern Pollings polling;
extern Tests tests[MAX_TEST];

extern uint8_t subnetCount, nodeSubnetCount;
extern uint16_t lastNetAddressClicked;
extern bool isOpenNodeControl;
extern PollingData pollingData;
extern GroupDataConfiguration groupDataConfiguration;
extern uint16_t timerGroupAddress[3];
extern CommissionData commissionData;
extern ScannedUUID scannedUUID[20];
extern uint8_t numberOfAddedNodes;
extern uint16_t numberOfIterations;
extern uint16_t netAddress;

extern QTimer pollingTimer;
extern QTimer testTimer;
extern QTimer groupFrameTimer;
extern QTimer addDeviceTimer;
extern QTimer confirmAddDeviceTimer;
extern QTimer newIterationTimer;

#endif // GLOBAL_VARIABLES_H
