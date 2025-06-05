#ifndef PROCESS_UART_DATA_H
#define PROCESS_UART_DATA_H

#include <QObject>
#include "UartPort.h"
#include "WebServer.h"
#include  "Database.h"
#include "global_variables.h"

#define UART_HEADER                     0xAF
#define UART_END                        0xAD
#define UART_CONFIG_FRAME_TYPE          0x01
#define UART_DALI_FRAME_TYPE            0x02
#define UART_POLLING_FRAME_TYPE         0x04
#define UART_ID_FRAME_TYPE              0x06
#define UART_RSP_CONFIG_FRAME_TYPE      0x11
#define UART_RSP_CHANGE_FRAME_TYPE      0x13
#define UART_RSP_POLLING_FRAME_TYPE     0x14

#define SCAN_DEVICES                    0x01
#define START_COMMISSION                0x03
#define NEW_ITERATION                   0x05
#define CHANGE_RELAY                    0x07
#define ADD_DEVICE                      0x09
#define DEVICE_ERROR                    0x11
#define DEL_DEVICES                     0x13
#define NODE_DELETED                    0x50
#define ADD_GROUP                       0x15
#define ADD_GROUP_MANUAL                0x16
#define DEL_GROUP                       0x17
#define CLEAR_ALL_DATA                  0x18
#define FEATURES                        0x19
#define WRITE_ID_CODE                   0x33
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
#define CONFIRM_START_SCAN              0x49
#define CONFIRM_START_REMOVE_ALL_NODES  0x51
#define CONFIRM_END_REMOVE_ALL_NODES    0x53
#define CONFIRM_ADD_NODE_TO_GROUP       0x55
#define CONFIRM_SET_POWER_ON_LEVEL      0x57
#define CONFIRM_START_REMOVE_ONE_NODE   0x59
#define CONFIRM_END_REMOVE_ONE_NODE     0x61
#define RELAY_STATUS                    0x63
#define SET_RELAY                       0x65
#define SCAN_FROM_NODE                  0x67
#define SCAN_NODE_NOT_FOUND             0x68
#define SET_ANTENNA_ADDRESS             0x69
#define CONFIRM_SET_ANTENNA_ADDRESS     0x70
#define GET_ANTENNA_ADDRESS             0x71
#define CONFIRM_GET_ANTENNA_ADDRESS     0x72
#define SEND_RECOVERY_NODE              0x73
#define CONFIRM_END_LINE_SCANNING       0x74
#define CONFIRM_START_LINE_SCANNING     0x75
#define SEND_FEATURES_STATUS            0x77
#define LS_GROUPS_RECOVERY              0x79
#define CONFIRM_START_GROUPS_RECOVERY   0x81
#define CONFIRM_END_GROUPS_RECOVERY     0x83

void processUartData(QByteArray data, WebServer* webServer, UartPort* uartPort, Database* database);
void processFeaturesFrame(QByteArray data, UartPort* uartPort, Database* database, WebServer* webServer);
void processGroupAddedFrame(QByteArray data, UartPort* uartPort, Database* database, WebServer* webServer);
void processChangeFrame(QByteArray data, Database* database, WebServer* webServer);
void processPollingFrame(QByteArray data);
void processConfirmGroupFrame(void);

void sendUartScannedDevices(UartPort* _uartPort);
void sendUartStartCommission(UartPort* _uartPort);
void sendUartNewIteration(UartPort* _uartPort);
void sendUartChangeRelay(UartPort* _uartPort, uint16_t nodeAddress);
void sendUartAddDevice(UartPort* _uartPort, ScannedUUID uuidScanned);
void sendUartDelDevice(UartPort* _uartPort, uint16_t nodeAddress);
void sendUartAddGroup(UartPort* _uartPort, uint16_t* address);
void sendUartAddGroupManual(UartPort* _uartPort, uint16_t* address);
void sendUartDelGroup(UartPort* _uartPort, uint16_t* address, Database* database);
void sendUartDelGroupForAllNodes(UartPort* _uartPort, uint16_t groupAddress, Database* database);
void sendUartDaliCommand(UartPort* _uartPort, uint16_t targetAddress, uint8_t daliRegister1, uint8_t daliRegister2, uint8_t commandType);
void sendPollingFrame(UartPort* _uartPort, uint16_t nodeAddress);
void sendWriteIDCodeFrame(UartPort* _uartPort, QString factoryCode);
void sendUartClearAllData(UartPort* _uartPort);
void sendUartPOLForUpdate(UartPort* _uartPort, Database* database);
void sendUartSetRelay(UartPort* _uartPort, uint16_t nodeAddress, bool enable);
void sendUartScanFromNode(UartPort* _uartPort, uint16_t nodeRealAddress);
void sendAntennaGetAddress(UartPort* _uartPort);
void sendAntennaSetAddress(UartPort* _uartPort, uint16_t newAntennaRealAddress);
void sendUartLineScanning(UartPort* _uartPort);
void processRecoveryFeaturesFrame(QByteArray data, Database* database);
void sendUartLSGroups(UartPort* _uartPort);
#endif // PROCESS_UART_DATA_H
