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
#define DEL_DEVICE                      0x13
#define ADD_GROUP                       0x15
#define DEL_GROUP                       0x17
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
void sendUartDelGroup(UartPort* _uartPort, uint16_t* address, Database* database);
void sendUartDaliCommand(UartPort* _uartPort, uint16_t targetAddress, uint8_t daliRegister1, uint8_t daliRegister2, uint8_t commandType);
void sendPollingFrame(UartPort* _uartPort, uint16_t nodeAddress);
void sendWriteIDCodeFrame(UartPort* _uartPort, QString factoryCode);

#endif // PROCESS_UART_DATA_H
