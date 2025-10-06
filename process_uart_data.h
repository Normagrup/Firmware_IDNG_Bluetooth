#ifndef PROCESS_UART_DATA_H
#define PROCESS_UART_DATA_H

#include <QObject>
#include "UartPort.h"
#include "WebServer.h"
#include  "Database.h"
#include "global_variables.h"
#include "UdpSocket.h"



#define UART_HEADER                     0xAF
#define UART_END                        0xAD
#define UART_CONFIG_FRAME_TYPE          0x01
#define UART_DALI_FRAME_TYPE            0x02
#define UART_POLLING_FRAME_TYPE         0x04
#define UART_ID_FRAME_TYPE              0x06
#define UART_RSP_CONFIG_FRAME_TYPE      0x11
#define UART_RSP_CHANGE_FRAME_TYPE      0x13
#define UART_RSP_POLLING_FRAME_TYPE     0x14

#define GROUP_FAIL                      0x01
#define DEV_TYPE_FAIL                   0x02
#define NET_ADDR_FAIL                   0x03

#define INYECT_NODE                     0xF1
#define CLEAR_INYECTED_NODES            0xF2
#define CLEAR_ONE_INYECTED_NODE         0xF3

#define UPDATE_NEXT_UNICAST             0xF0

#define SCAN_DEVICES                    0x01
#define START_COMMISSION                0x03
#define NEW_ITERATION                   0x05
#define CHANGE_RELAY                    0x07
#define ADD_DEVICES                     0x09
#define DEVICE_ERROR                    0x11
#define DEL_DEVICES                     0x13
#define NODE_DELETED                    0x50
#define ADD_GROUP                       0x15
#define ADD_GROUP_MANUAL                0x16
#define DEL_GROUP                       0x17
#define CLEAR_ALL_DATA                  0x18
#define FEATURES                        0x19
#define CONFIRM_ERROR_RETRY             0x32
#define WRITE_ID_CODE                   0x33
#define CONFIRM_RETRY                   0x34
#define FACTORY_ID_WROTE                0x35
#define DALI_TESTED                     0x37
#define RECORDED_DEVICE                 0x39
#define GROUP_ADDED                     0x21
#define CONFIRM_START_COMMISSION        0x23
#define CONFIRM_ADD_DEVICE              0x25
#define CONFIRM_GROUP_FRAME             0x27
#define CONFIRM_NEW_ITERATION           0x29
#define CONFIRM_CHANGE_RELAY            0x31
#define DEBUG                           0x90
#define LINE_SCANNING                   0x22
#define LINE_SCAN_SEND                  0x24
#define QUERY_RESPONSE                  0x47
#define CONFIRM_START_SCAN              0x49
#define CONFIRM_START_REMOVE_ALL_NODES  0x51
#define CONFIRM_END_REMOVE_ALL_NODES    0x53
#define CONFIRM_ADD_NODE_TO_GROUP       0x55
#define CONFIRM_DEL_NODE_FROM_GROUP     0x56
#define CONFIRM_SET_POWER_ON_LEVEL      0x57
#define CONFIRM_START_REMOVE_ONE_NODE   0x59
#define CONFIRM_END_REMOVE_ONE_NODE     0x61
#define RELAY_STATUS                    0x63
#define SET_RELAY                       0x65
#define SCAN_FROM_NODE                  0x67
#define SCAN_NODE_NOT_FOUND             0x68
#define SEND_RECOVERY_NODE              0x73
#define CONFIRM_END_LINE_SCANNING       0x74
#define CONFIRM_START_LINE_SCANNING     0x75
#define SEND_FEATURES_STATUS            0x77
#define ASK_POWER_ON_LEVEL              0x80
#define CONFIRM_START_GROUPS_RECOVERY   0x81
#define ANSWER_POWER_ON_LEVEL           0x82
#define CONFIRM_END_GROUPS_RECOVERY     0x83
#define COMMISSION_FAIL                 0x84
#define RECOVERY_GROUPS                 0x85
#define CHANGE_FATHER                   0x87
#define CONFIRM_END_CLEAR_ALL_DATA      0x88
#define SET_ANTENNA_ADDRESS_AND_NET_KEY 0xAA
#define GET_ANTENNA_ADDRESS_AND_NET_KEY 0xAB
#define ADDRESS_AND_NET_KEY_ANSWER      0xAC
#define ASK_INIT_DATA                   0xAD
#define START_LINE_SCANNING             0x91
#define END_LINE_SCANNING               0x92
#define ADDRESS_AND_NET_KEY             0x93
#define CONFIRM_REPLACE                 0x94
#define CONFIRM_REPLACE_DONE            0x95
#define CONFIRM_INYECT                  0x97
#define CONFIRM_ACTION                  0x98
#define CONFIRM_RESET_CDB               0x99

void processUartData(QByteArray data, WebServer* webServer, UartPort* uartPort, Database* database);
void extractAndProcessFrames(const QByteArray& rawData, WebServer* webServer, UartPort* uartPort, Database* database);
void processFeaturesFrame(QByteArray data, UartPort* uartPort, Database* database, WebServer* webServer);
void processGroupAddedFrame(QByteArray data, UartPort* uartPort, Database* database, WebServer* webServer);
void processChangeFrame(QByteArray data, Database* database, WebServer* webServer);
void processPollingFrame(QByteArray data);
void processConfirmGroupFrame(void);
void processRecoveryFeaturesFrame(QByteArray data, Database* database);
int getExpectedFrameSize(const QByteArray& buffer);

void sendUartInyectNode(UartPort* _uartPort, uint16_t nodeAddress, Database* database);
void sendUartClearInyectedNodes(UartPort* _uartPort, bool isCommissioning, Database* database);
void sendUartClearOneInyectedNode(UartPort* _uartPort, uint16_t nodeAddress);

void sendUartUpdateNextUnicast(UartPort* _uartPort, uint16_t nextUnicastAddress);

void sendUartScannedDevices(UartPort* _uartPort);
void sendUartStartCommission(UartPort* _uartPort, uint16_t nextUnicastAddress);
void sendUartNewIteration(UartPort* _uartPort, uint16_t addressToNextIt);
void sendUartChangeRelay(UartPort* _uartPort, uint16_t nodeAddress, Database* database);
void sendUartAddDevice(UartPort* _uartPort, ScannedUUID uuidScanned);
void sendUartDelDevice(UartPort* _uartPort, uint16_t nodeAddress, bool isBroadcast);
void sendUartAddGroup(UartPort* _uartPort, uint16_t* address);
void sendUartAddGroupManual(UartPort* _uartPort, uint16_t* address);
void sendUartDelGroup(UartPort* _uartPort, uint16_t* address, Database* database);
void sendUartDelGroupSimple(UartPort* _uartPort, uint16_t* address); // no actualiza el modelo y la bbdd directamente
void sendUartDelGroupForAllNodes(UartPort* _uartPort, uint16_t groupAddress, Database* database);
void sendUartDelGroupForAllNodesUnitary(UartPort* _uartPort, uint16_t nodeRealAddress, uint16_t groupAddress);
void sendUartDaliCommand(UartPort* _uartPort, uint16_t targetAddress, uint8_t daliRegister1, uint8_t daliRegister2, uint8_t commandType);
void sendPollingFrame(UartPort* _uartPort, uint16_t nodeAddress);
void sendWriteIDCodeFrame(UartPort* _uartPort, QString factoryCode);
void sendUartClearAllData(UartPort* _uartPort, uint16_t nodeAddress);
void sendUartPOLForUpdate(UartPort* _uartPort, Database* database);
void sendUartPOLForUpdateUnitary(UartPort* _uartPort, uint16_t realAddress);
void sendUartSetRelay(UartPort* _uartPort, uint16_t nodeAddress, bool enable);
void sendUartScanFromNode(UartPort* _uartPort, uint16_t nodeRealAddress);
void sendSetAntennaAddressAndNetKey(UartPort* _uartPort, Database* _database);
void sendGetAntennaAddressAndNetKey(UartPort* _uartPort);
void sendAntennaAddressAndNetKey(UartPort* _uartPort, bool antennaIDHasChanged, bool netKeyHasChanged);
void sendUartStartLineScanning(UartPort* _uartPort);
void sendUartLineScanning(UartPort* _uartPort, uint8_t phase, uint16_t nodeAddress);
void sendUartEndLineScanning(UartPort* _uartPort);
void sendUartChangeFather(UartPort* _uartPort, uint16_t childRealAddress, uint16_t fatherRealAddress);
void processRecoveryFeaturesFrame(QByteArray data, Database* database);
void sendPowerOnLeveltoEth(uint16_t pid, uint8_t powerOnLevel, QString rcvAddress, UdpSocket* _udpSocket);
void sendUartConfirmReplacing(UartPort* _uartPort, uint16_t realAddress);

#endif // PROCESS_UART_DATA_H
