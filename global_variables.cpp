#include "global_variables.h"

WebServerData webServerData = {"", "", "", "", "", "", ""};
QString networkIP[4] = {"", "", "", ""};
QString networkSubmask[4] = {"", "", "", ""};
QString gatewayAddress[4] = {"", "", "", ""};
QString macAddress[6] = {"", "", "", "", "", ""};

Device meshDevice[MAX_SUBNET][MAX_NODES_SUBNET];
//ScannedUUID scannedUUID[MAX_SUBNET * MAX_NODES_SUBNET] = { {nullptr, 0} };
Pollings polling;
Tests tests[MAX_TEST];
QTimer pollingTimer;
QTimer testTimer;
QTimer groupFrameTimer;
QTimer addDeviceTimer;
QTimer confirmAddDeviceTimer;
QTimer newIterationTimer;
uint16_t lastNetAddressClicked = 0;
ScannedUUID scannedUUID[20];
bool isOpenNodeControl = false;
PollingData pollingData = {false, false, 0};
GroupDataConfiguration groupDataConfiguration = {true, false};
uint16_t timerGroupAddress[3] = {0};
CommissionData commissionData = {0};
uint8_t numberOfAddedNodes = 0;
uint16_t numberOfIterations = 0;
uint16_t netAddress = 0;

uint8_t subnetCount = 0, nodeSubnetCount = 0;
