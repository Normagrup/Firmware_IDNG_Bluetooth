#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

#include <QTimer>
#include <QMap>

#include "structures.h"
#include "Device.h"
#include "Pollings.h"
#include "Tests.h"

#define MAX_SUBNET              32
#define MAX_NODES_SUBNET        64
#define MAX_TEST                37 // Grupo de Broadcast, 4 grupos por defecto, 32 grupos extra (PROVISIONAL)

extern WebServerData webServerData;
extern QString networkIP[4];
extern QString networkSubmask[4];
extern QString gatewayAddress[4];
extern QString macAddress[6];

extern Device meshDevice[MAX_SUBNET][MAX_NODES_SUBNET];
//extern ScannedUUID scannedUUID[MAX_SUBNET * MAX_NODES_SUBNET];
extern Pollings polling;
extern Tests tests[MAX_TEST];
extern QList<AntennaTestCheck> antennaTestCheckList;

extern uint16_t antennaRealAddress;

extern bool isCommissioning; // logic to prevent new commands
extern bool logsSavedToday; // to check if today´s log is saved

extern uint8_t subnetCount, nodeSubnetCount;
extern uint16_t lastNetAddressClicked;
extern bool isOpenNodeControl;
extern PollingData pollingData;
extern GroupDataConfiguration groupDataConfiguration;
extern uint16_t timerGroupAddress[3];
extern CommissionData commissionData;
extern ScannedUUID scannedUUID[20];
extern ScannedUUID scannedUUIDBackup[20];
extern uint8_t numberOfAddedNodes;
extern uint16_t numberOfIterations;
extern uint16_t doneIterations;
extern uint16_t netAddress;
extern uint16_t currentNodeAddress;

extern QTimer pollingTimer;
extern QTimer testResultCheckTimer;
extern QTimer testTimer;
extern QTimer groupFrameTimer;
extern QTimer addDeviceTimer;
extern QTimer confirmAddDeviceTimer;
extern QTimer newIterationTimer;
extern QList<QPair<uint16_t, QStringList>> crossedGroupAndNodes;

extern bool forceStopCommissioning;
extern bool isManualAddingDevice;
extern bool isScanning;
extern uint16_t lineScanningCounter; // para calcular la posición del siguiente nodo escaneado
extern uint16_t scannedNodesCounter; // para mostrar en el WS la cantidad de nodos encontrados
extern QList<uint16_t> configuredNodes;
extern QMap<uint16_t, NodeInfo> nodesByRealAddress;
extern QMultiMap<uint16_t, uint16_t> childrenMap;

extern bool isLineScanning;
extern uint8_t failComCycles;

extern QString reportType;
extern QString startDate;
extern QString endDate;

extern uint8_t netKeys[15][16];

#endif // GLOBAL_VARIABLES_H
