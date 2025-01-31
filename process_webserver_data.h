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

#define WS_SET_SCANNED_DEVICES              "SET_SCANNED_DEVICES"
#define WS_SET_START_ACTION                 "SET_START_ACTION"
#define WS_SET_NEW_COMMISSION_ITERATION     "SET_NEW_COMMISSION_ITERATION"
#define WS_SET_DELETE_DEVICE                "SET_DELETE_DEVICE"
#define WS_SET_ADD_GROUP                    "SET_ADD_GROUP"
#define WS_SET_DEL_GROUP                    "SET_DEL_GROUP"
#define WS_SET_LOAD_NODES                   "SET_LOAD_NODES"
#define WS_SET_TEST                         "SET_TEST"
#define WS_SET_UPDATE_FILE                  "SET_UPDATE_FILE"
#define WS_GET_LOGS                         "GET_LOGS"
#define WS_GET_NODE_INFO                    "GET_NODE_INFO"
#define WS_SET_CLOSE_CONTROL                "SET_CLOSE_CONTROL"
#define WS_SET_READ_ID_CODE                 "SET_READ_ID_CODE"

#define WS_SET_MAX                          "SET_MAX"
#define WS_SET_OFF                          "SET_OFF"
#define WS_SET_MIN                          "SET_MIN"
#define WS_SET_RESET                        "SET_RESET"
#define WS_SET_ACTUAL_LVL                   "SET_ACTUAL_LVL"
#define WS_SET_IDENTIFY                     "SET_IDENTIFY"
#define WS_SET_FACTORY_SETTINGS             "SET_FACTORY_SETTINGS"
#define WS_SET_REBOOT                       "SET_REBOOT"
#define WS_SET_FUNCTION_TEST                "SET_FUNCTION_TEST"
#define WS_SET_DURATION_TEST                "SET_DURATION_TEST"
#define WS_SET_STOP                         "SET_STOP"

#define WS_SEND_LOGIN_INFO                  "LOG_IN_INFO"
#define WS_SEND_INTERFACES_INFO             "INTERFACES_INFO"
#define WS_SEND_DATE_TIME_INFO              "DATE_TIME_INFO"
#define WS_SEND_IPCONFIG_INFO               "IPCONFIG_INFO"
#define WS_SEND_CONFIRM_START_COMMISSION    "CONFIRM_START_COMMISSION"
#define WS_SEND_CONFIRM_ADDING_DEVICE       "CONFIRM_ADDING_DEVICE"
#define WS_SEND_START_ADDING_DEVICES        "START_ADDING_DEVICES"
#define WS_SEND_SCANNED_DEVICES             "SCANNED_DEVICE"
#define WS_SEND_ADDED_DEVICES               "ADDED_DEVICE"
#define WS_SEND_DEVICE_ERROR                "DEVICE_ERROR"
#define WS_SEND_LOADED_NODES                "LOADED_NODES"
#define WS_SEND_NODE_INFO                   "NODE_INFO"
#define WS_SEND_END_NODE_CONFIG             "END_NODE_CONFIG"
#define WS_SEND_END_AUTO_COMMISSION         "END_AUTO_COMMISSION"
#define WS_SEND_FACTORY_ID_WROTE            "FACTORY_ID_WROTE"
#define WS_SEND_DALI_TESTED                 "DALI_TESTED"
#define WS_SEND_RECORDED_DEVICE             "RECORDED_DEVICE"

void processWebServerData(QString data, WebServer* webServer, UartPort* uartPort, Database* database);

void sendLoginInfo(WebServer* webServer, uint8_t loginInfo);
void sendInterfaceInfo(WebServer* webServer, QString info);
void sendDateTimeInfo(WebServer* webServer, QString info);
void sendIPConfigInfo(WebServer* webServer, bool ipConfigInfo);
void sendConfirmStartCommission(WebServer* webServer);
void sendConfirmAddingDevice(WebServer* webServer);
void sendStartAddingDevices(WebServer* webServer);
void sendScannedDevices(QByteArray data, WebServer* webServer);
void sendAddedDevices(QByteArray data, WebServer* webServer, Database* database);
void sendDeviceError(QByteArray data, UartPort* uartPort, WebServer* webServer);
void sendNodesFromDatabase(WebServer* webServer, Database* database);
void sendNodeInfo(WebServer* webServer, QString nodeAddress);
void sendEndNodeConfiguration(WebServer* webServer);
void sendEndAutoCommission(WebServer* webServer);
void sendFactoryIDWrote(WebServer* webServer);
void sendDaliTested(WebServer* webServer);
void sendRecordedDevice(WebServer* webServer);

#endif // PROCESS_WEBSERVER_DATA_H
