#ifndef PROCESS_WEBSERVER_DATA_H
#define PROCESS_WEBSERVER_DATA_H

#include <QObject>
#include "UartPort.h"
#include "WebServer.h"
#include "Database.h"

#define WS_SET_LOG_IN                           "SET_LOG_IN"
#define WS_SET_REBOOT_DEVICE                "SET_REBOOT_DEVICE"
#define WS_GET_IP_CONFIG                    "GET_IP_CONFIG"
#define WS_SET_IP_CONFIG                    "SET_IP_CONFIG"
#define WS_GET_DATE_TIME                    "GET_DATE_TIME"
#define WS_SET_DATE_TIME                    "SET_DATE_TIME"
#define WS_ASK_STATE_TO_EMBEDDED            "ASK_STATE_TO_EMBEDDED"

#define WS_SET_SCANNED_DEVICES              "SET_SCANNED_DEVICES"
#define WS_SET_STORED_SCANNED_DEVICES       "SET_STORED_SCANNED_DEVICES"
#define WS_SET_START_ACTION                 "SET_START_ACTION"
#define WS_SET_NEW_COMMISSION_ITERATION     "SET_NEW_COMMISSION_ITERATION"
#define WS_SET_STOP_ACTION                  "SET_STOP_ACTION"
#define WS_SET_DELETE_DEVICE                "SET_DELETE_DEVICE"
#define WS_SET_ADD_DEVICE                   "SET_ADD_DEVICE"
#define WS_SET_ADD_GROUP                    "SET_ADD_GROUP"     // añadir nodo a grupo
#define WS_SET_ADD_A_GROUP                  "SET_ADD_A_GROUP"   // añadir un grupo
#define WS_SET_DEL_GROUP                    "SET_DEL_GROUP"     // eliminar nodo de grupo)
#define WS_SET_DEL_A_GROUP                  "SET_DEL_A_GROUP"   // eliminar un grupo
#define WS_SET_EDIT_A_GROUP                 "SET_EDIT_A_GROUP"  // cambiar nombre de grupo
#define WS_GET_GROUP_NODES                  "GET_GROUP_NODES"
#define WS_SET_LOAD_NODES                   "SET_LOAD_NODES"
#define WS_SET_TEST                         "SET_TEST"
#define WS_SET_UPDATE_FILE                  "SET_UPDATE_FILE"
#define WS_GET_LOGS                         "GET_LOGS"
#define WS_GET_LOGS_PAGED                   "GET_LOGS_PAGED"
#define WS_DOWNLOAD_LOGS                    "DOWNLOAD_LOGS"
#define WS_GET_NODE_INFO                    "GET_NODE_INFO"
#define WS_SET_CLOSE_CONTROL                "SET_CLOSE_CONTROL"
#define WS_SET_READ_ID_CODE                 "SET_READ_ID_CODE"
#define WS_GET_DEVICES_COUNT                "GET_DEVICES_COUNT"
#define WS_GET_FAILURES_COUNT               "GET_FAILURES_COUNT"
#define WS_GET_IS_CONFIG                    "GET_IS_CONFIG"
#define WS_GET_GROUPS                       "GET_GROUPS"
#define WS_GET_GROUP_INFO                   "GET_GROUP_INFO"
#define WS_GET_TEST                         "GET_TEST"
#define WS_SET_CLEAR_ALL_DATA               "SET_CLEAR_ALL_DATA"
#define WS_LINE_SCANNING                    "LINE_SCANNING"
#define WS_GET_POWER_ON_LEVEL               "GET_POWER_ON_LVL"
#define WS_SET_POWER_ON_LEVEL               "SET_POWER_ON_LVL"
#define WS_SET_SYNC_POL                     "SET_SYNC_POL"
#define WS_GET_MASTER_REAL_ADDRESS          "GET_MASTER_REAL_ADDRESS"
#define WS_GET_FAILCOM_CYCLES               "GET_FAILCOM_CYCLES"
#define WS_SET_FAILCOM_CYCLES               "SET_FAILCOM_CYCLES"
#define WS_GET_INSTALL_KEY                  "GET_INSTALL_KEY"
#define WS_SET_INSTALL_KEY                  "SET_INSTALL_KEY"
#define WS_GET_ACTIVE_KEY_AND_FORCE         "GET_ACTIVE_KEY_AND_FORCE"
#define WS_SET_ACTIVE_KEY                   "SET_ACTIVE_KEY"
#define WS_CHANGE_NODES                     "CHANGE_NODES"
#define WS_GET_LINE_SCANNED_NODES           "GET_LINE_SCANNED_NODES"
#define WS_STOP_LS                          "STOP_LS"
#define WS_SET_MASTER_ADDR_AND_INSTALLKEY   "SET_MASTER_ADDR_AND_INSTALLKEY"
#define WS_REPLACE_NODES                    "REPLACE_NODES"
#define WS_ADD_UNASSIGNED_NODE              "ADD_UNASSIGNED_NODE"
#define WS_GET_UNASSIGNED_NODES_PAGED       "GET_UNASSIGNED_NODES_PAGED"
#define WS_AUTOASSIGNMENT                   "AUTOASSIGNMENT"
#define WS_APPLY_AUTOASSIGNMENT             "APPLY_AUTOASSIGNMENT"
#define WS_DEL_UNASSIGNED_NODE              "DEL_UNASSIGNED_NODE"
#define WS_GROUP_AUTOASSIGNMENT             "GROUP_AUTOASSIGNMENT"
#define WS_UPDATE_UNASSIGNED                "UPDATE_UNASSIGNED"

#define WS_SET_MAX                          "SET_MAX"
#define WS_SET_OFF                          "SET_OFF"
#define WS_SET_MIN                          "SET_MIN"
#define WS_SET_RESET                        "SET_RESET"
#define WS_SET_ACTUAL_LVL                   "SET_ACTUAL_LVL"
#define WS_SET_IDENTIFY                     "SET_IDENTIFY"
#define WS_STOP_IDENTIFY                    "STOP_IDENTIFY"
#define WS_SET_FACTORY_SETTINGS             "SET_FACTORY_SETTINGS"
#define WS_SET_REBOOT                       "SET_REBOOT"
#define WS_SET_FUNCTION_TEST                "SET_FUNCTION_TEST"
#define WS_SET_DURATION_TEST                "SET_DURATION_TEST"
#define WS_SET_STOP                         "SET_STOP"
#define WS_SET_SCAN_FROM_NODE               "SET_SCAN_FROM_NODE"
#define WS_SET_RELAY_MODE                   "SET_RELAY_MODE"
#define WS_SEND_RECOVERING_MICRO            "IS_RECOVERING_MICRO"
#define WS_SEND_LOGIN_INFO                  "LOG_IN_INFO"
#define WS_SEND_INTERFACES_INFO             "INTERFACES_INFO"
#define WS_SEND_DATE_TIME_INFO              "DATE_TIME_INFO"
#define WS_SEND_IPCONFIG_INFO               "IPCONFIG_INFO"
#define WS_SEND_LOG_COMMISSION_ENTRY        "LOG_COMMISSION_ENTRY"
#define WS_SEND_CONFIRM_START_SCAN          "CONFIRM_START_SCAN"
#define WS_SEND_CONFIRM_END_SCAN            "CONFIRM_END_SCAN"
#define WS_SEND_CONFIRM_START_COMMISSION    "CONFIRM_START_COMMISSION"
#define WS_SEND_CONFIRM_ADDING_DEVICE       "CONFIRM_ADDING_DEVICE"
#define WS_SEND_START_ADDING_DEVICES        "START_ADDING_DEVICES"
#define WS_SEND_SCANNED_DEVICES             "SCANNED_DEVICE"
#define WS_SEND_ADDED_DEVICES               "ADDED_DEVICE"
#define WS_SEND_CONFIRM_ADD_DEVICE          "CONFIRM_ADD_DEVICE"
#define WS_SEND_DEVICE_ERROR                "DEVICE_ERROR"
#define WS_SEND_LOADED_NODES                "LOADED_NODES"
#define WS_SEND_NODE_INFO                   "NODE_INFO"
#define WS_SEND_GROUP                       "GROUP_NAME_AND_ADDRESS"
#define WS_SEND_GROUP_INFO                  "GROUP_INFO"
#define WS_SEND_GROUP_WITH_POL              "GROUP_WITH_POL"
#define WS_SEND_GROUP_NODE_INCLUDED         "GROUP_NODE_INCLUDED"
#define WS_SEND_GROUP_NODE_NOT_INCLUDED     "GROUP_NODE_NOT_INCLUDED"
#define WS_SEND_CONFIRM_ADD_NODE_TO_GROUP   "CONFIRM_ADD_NODE_TO_GROUP"
#define WS_SEND_CONFIRM_DEL_NODE_FROM_GROUP "CONFIRM_DEL_NODE_FROM_GROUP"
#define WS_SEND_CONFIRM_DEL_GROUP           "CONFIRM_DEL_GROUP"
#define WS_SEND_CONFIRM_MANUAL_RELAY        "CONFIRM_MANUAL_RELAY"
#define WS_SEND_CONFIRM_POWER_ON_LEVEL      "CONFIRM_POWER_ON_LEVEL"
#define WS_SEND_TEST                        "TEST_DATA"
#define WS_SEND_DEVICES_COUNTER             "DEVICES_COUNTER"
#define WS_SEND_FAILURES_COUNTER            "FAILURES_COUNTER"
#define WS_SEND_END_NODE_CONFIG             "END_NODE_CONFIG"
#define WS_SEND_END_AUTO_COMMISSION         "END_AUTO_COMMISSION"
#define WS_SEND_FACTORY_ID_WROTE            "FACTORY_ID_WROTE"
#define WS_SEND_DALI_TESTED                 "DALI_TESTED"
#define WS_SEND_RECORDED_DEVICE             "RECORDED_DEVICE"
#define WS_SEND_SERIAL_CLOSURE              "SERIAL_CLOSURE"
#define WS_SEND_IS_CONFIG                   "IS_CONFIG"
#define WS_SEND_LOG_DATA                    "LOG_DATA"
#define WS_SEND_LOG_FILE                    "LOG_FILE"
#define WS_SEND_CONFIRM_START_DEL_ALL_DEV   "CONFIRM_START_DEL_ALL_DEV"
#define WS_SEND_CONFIRM_END_DEL_ALL_DEV     "CONFIRM_END_DEL_ALL_DEV"
#define WS_SEND_CONFIRM_START_DEL_ONE_DEV   "CONFIRM_START_DEL_ONE_DEV"
#define WS_SEND_CONFIRM_END_DEL_ONE_DEV     "CONFIRM_END_DEL_ONE_DEV"
#define WS_SEND_CONFIRM_SET_RELAY           "CONFIRM_SET_RELAY"
#define WS_SEND_CONFIRM_START_LS            "CONFIRM_START_LS"
#define WS_SEND_CONFIRM_END_LS              "CONFIRM_END_LS"
#define WS_SEND_CONFIRM_END_SYNC_POL        "CONFIRM_END_SYNC_POL"
#define WS_SEND_LS_INFO                     "LS_INFO"
#define WS_SEND_LS_FOUNDED                  "LS_FOUNDED"
#define WS_SEND_CONFIRM_END_CLEAR_ALL       "CONFIRM_END_CLEAR_ALL"
#define WS_SEND_CONFIRM_START_REPLACE       "CONFIRM_START_REPLACE"
#define WS_SEND_CONFIRM_END_REPLACE         "CONFIRM_END_REPLACE"
#define WS_SEND_WRITE_ID_ERROR              "WRITE_ID_ERROR"
#define WS_SEND_ESTIMATED_TIME              "ESTIMATED_TIME"
#define WS_SEND_INIT_ALERT                  "INIT_ALERT"
#define WS_SEND_UNASSIGNED_NODES            "UNASSIGNED_NODES"
#define WS_SEND_START_APPLY_ASSIGN          "START_APPLY_ASSIGN"
#define WS_SEND_END_APPLY_ASSIGN            "END_APPLY_ASSIGN"
#define WS_SEND_START_GROUP_AUTO_ASSIGN     "START_GROUP_AUTO_ASSIGN"
#define WS_SEND_INFO_GROUP_AUTO_ASSIGN      "INFO_GROUP_AUTO_ASSIGN"
#define WS_SEND_END_GROUP_AUTO_ASSIGN       "END_GROUP_AUTO_ASSIGN"
#define WS_SEND_ACTIVE_KEY_AND_FORCE        "ACTIVE_KEY_AND_FORCE"
#define WS_SEND_ADD_UNASSIGNED_ERROR        "ADD_UNASSIGNED_ERROR"
#define WS_SWITCH_FORCE                     "SWITCH_FORCE"
#define WS_BLINK_ASSIGNED                   "BLINK_ASSIGNED"
#define WS_BLINK_UNASSIGNED                 "BLINK_UNASSIGNED"
#define WS_BLINK_STOP                       "BLINK_STOP"
#define WS_BLINK_UNASSIGNED_NODE            "BLINK_UNASSIGNED_NODE"

#define WS_SEND_CONFIRM_M_ADDRESS_GET       "CONFIRM_M_ADDRESS_GET"
#define WS_SEND_INSTALL_KEY_GET             "INSTALL_KEY_GET"

#define WS_SEND_FAIL_COM_CYCLES             "FAIL_COM_CYCLES"


void processWebServerData(QString data, WebServer* webServer, UartPort* uartPort, Database* database);

void addNodeForReplace(WebServer* webServer, UartPort* uartPort, Database* database);
void deleteNodeForReplace(WebServer* webServer, UartPort* uartPort, Database* database);
void restoreDataForReplace(WebServer* webServer, UartPort* uartPort, Database* database);
void sendRecoveringMicro(WebServer* webServer);
void sendLoginInfo(WebServer* webServer, uint8_t loginInfo);
void sendInterfaceInfo(WebServer* webServer, QString info);
void sendDateTimeInfo(WebServer* webServer, QString info);
void sendIPConfigInfo(WebServer* webServer, bool ipConfigInfo);
void sendLogCommissionEntry(WebServer* webServer, QString content, QString type);
void sendConfirmStartScan(WebServer* webServer);
void sendConfirmEndScan(WebServer* webServer);
void sendConfirmStartCommission(WebServer* webServer);
void sendConfirmAddingDevice(WebServer* webServer);
void sendDevError(WebServer* webServer);
void sendStartAddingDevices(WebServer* webServer);
void sendScannedDevices(QByteArray data, WebServer* webServer);
void sendStoredScannedDevices(WebServer* webServer);
void sendAddedDevices(QByteArray data, WebServer* webServer, Database* database);
void sendDeviceError(QByteArray data, UartPort* uartPort, WebServer* webServer, Database* database);
void sendNodesFromDatabase(WebServer* webServer, Database* database);
void sendNodeInfo(WebServer* webServer, QString nodeAddress);
void sendGroups(WebServer* webServer, Database* database);
void sendGroupInfo(WebServer* webServer, QString groupAddress);
void sendGroupNodes(WebServer* webServer, QString groupAddress);
void sendGroupsWithPOL(WebServer* webServer, Database* database, QString value);
void sendTest(WebServer* webServer, Database* database, QString groupAddress);
void sendDevicesCount(WebServer* webServer, int count);
void sendFailuresCount(WebServer* webServer, int count, int lampFailCounter, int batFailCounter, int durFailCounter, int comFailCounter);
void sendEndNodeConfiguration(WebServer* webServer);
void sendEndAutoCommission(WebServer* webServer);
void sendFactoryIDWrote(WebServer* webServer, bool received);
void sendDaliTested(WebServer* webServer, bool received);
void sendRecordedDevice(WebServer* webServer, bool received);
void sendSerialClosure(WebServer* webServer, bool done);
void sendLogData(WebServer* webServer, QList<QStringList> logs);
void sendLogFile(WebServer* webServer, QString fileDir);
void clearSystemData(WebServer* webServer, Database* database, UartPort* uartPort);
void sendIsConfig(WebServer* webServer, QString device, QString serialNumber, bool isConfig, bool hasFailures, bool onOffStatus, bool isInEmergency);
void sendConfirmStartRemoveAllNodes(WebServer* webServer);
void sendConfirmEndRemoveAllNodes(WebServer* webServer);
void sendConfirmStartRemoveOneNode(WebServer* webServer);
void sendConfirmEndRemoveOneNode(WebServer* webServer);
void sendConfirmAddNodeToGroup(WebServer* webServer, uint16_t address, uint16_t deviceTypeGroupAddress, bool added, Database* database);
void sendConfirmDelNodeFromGroup(WebServer* webServer);
void sendConfirmDelGroup(WebServer* webServer);
void sendConfirmSetRelay(WebServer* webServer);
void sendConfirmPowerOnLevel(WebServer* webServer, uint8_t powerOnLevel, uint16_t groupAddress, Database* database);
void updateRelayStatus(WebServer* webServer, Database* database, uint16_t address, bool enabled);
void reloadAntennaAddressAndInstallKey(WebServer* webServer, Database* database, uint16_t antennaAddress, const uint8_t* installKey);
void sendFailComCycles(WebServer* webServer);
void updatePowerOnLevels(WebServer* webServer, Database* database, uint16_t nodeAddr, uint8_t powerOnLevel);
void sendConfirmStartLineScanning(WebServer* webServer);
void sendConfirmEndLineScanning(WebServer* webServer);
void sendConfirmEndSyncPOL(WebServer* webServer);
void sendLSInfo(WebServer* webServer, uint16_t nodeAddr, uint8_t phase);
void sendFoundNodes(WebServer* webServer, uint16_t nodesCount);
void changePositions(Database* database, uint16_t pos1, uint16_t pos2);
void sendConfirmEndClearAllData(WebServer* webServer);
void sendConfirmStartReplace(WebServer* webServer);
void sendConfirmEndReplace(WebServer* webServer);
void sendWriteIDError(WebServer* webServer);
void sendEstimatedTime(WebServer* webServer, uint16_t time);
void sendInitAlert(WebServer* webServer);
void sendIdentify(UartPort* uartPort, uint16_t nodeNetAddress);
void sendUnassignedNodesPaged(WebServer* webServer, Database* database, uint16_t page);
void applyAutoAssignment(WebServer* webServer, UartPort* uartPort, Database* database);
void sendConfirmStartApplyAutoAssignment(WebServer* webServer);
void sendConfirmEndApplyAutoAssignment(WebServer* webServer);
void sendConfirmStartGroupAutoAssignment(WebServer* webServer);
void applyGroupAutoAssignment(WebServer* webServer, UartPort* uartPort, Database* database);
void sendConfirmEndGroupAutoAssignment(WebServer* webServer);
void sendGroupAutoAssignInfo(WebServer* webServer, int counter, int totalNodes);
void sendActiveKeyAndForcing(WebServer* webServer, uint8_t activeKey, bool forceInstallKey);
void sendAddUnassignedError(WebServer* webServer, uint8_t result);

void processFactoryProgramSerial(UartPort* uartPort, QByteArray dataBuffer);

#endif // PROCESS_WEBSERVER_DATA_H
