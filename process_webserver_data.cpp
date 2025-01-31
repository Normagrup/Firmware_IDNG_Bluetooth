#include "process_webserver_data.h"
#include "process_uart_data.h"
#include "global_variables.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "time_functions.h"
#include "file_handler.h"

void processWebServerData(QString data, WebServer* webServer, UartPort* uartPort, Database* database)
{
    QStringList dataParts = data.split("@");
    QString type = dataParts[0].trimmed();
    QString value = dataParts[1].trimmed();

    qDebug() << "WEBSERVER FRAME RECEIVED: " << type;

    pollingTimer.stop();

    if (type == WS_SET_LOG_IN) {
        uint8_t loginInfo = database->verifyLoginParameters(value);
        sendLoginInfo(webServer, loginInfo);
    }
    else if (type == WS_SET_REBOOT_DEVICE) {
        rebootDevice();
    }
    else if (type == WS_SET_SCANNED_DEVICES) {
        sendUartScannedDevices(uartPort);
    }
    else if (type == WS_GET_IP_CONFIG) {
        QString message = database->getInterfaceParameters();
        sendInterfaceInfo(webServer, message);
    }
    else if (type == WS_SET_IP_CONFIG) {
        QStringList webServerParts = value.split(" ");
        QString password = webServerParts[5];

        QStringList databaseLoginParametersList = database->getLoginParameters();
        bool passwordFound = false;

        for (const QString& databaseEntry : databaseLoginParametersList) {
            QStringList databaseParts = databaseEntry.split(" ");
            if (databaseParts.size() > 1 && databaseParts[1] == "A1234") {
                passwordFound = (password == databaseParts[1]);
                break;
            }
        }

        if (passwordFound) {
            sendIPConfigInfo(webServer, true);
            setIPConfigInfo(webServerParts, database);
        }
        else { sendIPConfigInfo(webServer, false); }
    }
    else if (type == WS_GET_DATE_TIME) {
        QString date = getLocalDate();
        QString time = getLocalTime();
        QString message = date + " " + time;
        sendDateTimeInfo(webServer, message);
    }
    else if (type == WS_SET_DATE_TIME) {
        QStringList webServerParts = value.split(" ");
        setLocalDateTime(webServerParts);
    }
    else if (type == WS_SET_START_ACTION) {
        //qDebug() << "ADDING NEW NODE";
        if (value != "0") {
            sendUartDelDevice(uartPort, 0x0000);
            //uuidScanned = compareDeviceUUID(value);
            //delay(500);
            //confirmAddDeviceTimer.start(CONFIRM_ADD_DEVICE_TIMER_MS);
            //if (uuidScanned.UUID != nullptr) { sendUartAddDevice(uartPort, uuidScanned); }
        }
        else {
            qDebug() << "START COMMISSION";
            sendUartStartCommission(uartPort);
        }
    }
    else if (type == WS_SET_NEW_COMMISSION_ITERATION) {
        /*
        qDebug() << "NEW ITERATION" << commissionData.numberOfNodesScanned;
        if (commissionData.numberOfNodesScanned > 0) { sendUartNewIteration(uartPort); }
        else {
            sendEndAutoCommission(webServer);
            pollingTimer.start(POLLING_TIMER_MS);
        }
        */
    }
    else if (type == WS_SET_DELETE_DEVICE) {
        //uint16_t nodeNetAddress = getNodeNetAddress(value);
        //uint16_t  nodeAddress = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
        //meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].deleteDevice();
        //database->deleteNode(nodeAddress);
        sendUartDelDevice(uartPort, 0xFFFF);
    }
    else if (type == WS_SET_ADD_GROUP) {
        /*
        uint16_t* address = getGroupAddress(value);
        timerGroupAddress[0] = address[0];
        timerGroupAddress[1] = address[1];
        groupDataConfiguration.configSecondGroup = false;
        qDebug() << "GROUP ADD";
        sendUartAddGroup(uartPort, address);
        */
    }
    else if (type == WS_SET_DEL_GROUP) {
        uint16_t* address = getGroupAddress(value);
        sendUartDelGroup(uartPort, address, database);
    }
    else if (type == WS_SET_MAX) {
        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, RECALL_MAX_LVL, IS_NORMAL);
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, RECALL_MAX_LVL, IS_NORMAL);
        }
    }
    else if (type == WS_SET_OFF) {
        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, OFF, IS_NORMAL);
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, OFF, IS_NORMAL);
        }
    }
    else if (type == WS_SET_MIN) {
        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, RECALL_MIN_LVL, IS_NORMAL);
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, RECALL_MIN_LVL, IS_NORMAL);
        }
    }
    else if (type == WS_SET_RESET) {
        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, RESET, IS_TWICE);
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, RESET, IS_TWICE);
        }
    }
    else if (type == WS_SET_ACTUAL_LVL) {
        uint16_t* values = getActualLvl(value);
        sendUartDaliCommand(uartPort, values[0], ARC_POWER_DAPC, values[1], IS_NORMAL);
    }
    else if (type == WS_SET_IDENTIFY) {
        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, IDENTIFY_DEVICE, IS_TWICE);
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, IDENTIFY_DEVICE, IS_TWICE);
        }
    }
    else if (type == WS_SET_FACTORY_SETTINGS) {
        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            sendUartDaliCommand(uartPort, nodeAddress, DTR_0, 0x63, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeAddress, DTR_1, 0x63, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, ENABLE_WRITE_MEMORY, IS_TWICE);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeAddress, WRITE_MEMORY_LOCATION, 0x02, IS_NORMAL);
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, DTR_0, 0x63, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, DTR_1, 0x63, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, ENABLE_WRITE_MEMORY, IS_TWICE);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, WRITE_MEMORY_LOCATION, 0x02, IS_NORMAL);
        }
    }
    else if (type == WS_SET_REBOOT) {
        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            sendUartDaliCommand(uartPort, nodeAddress, DTR_0, 0x63, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeAddress, DTR_1, 0x63, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, ENABLE_WRITE_MEMORY, IS_TWICE);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeAddress, WRITE_MEMORY_LOCATION, 0x01, IS_NORMAL);
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, DTR_0, 0x63, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, DTR_1, 0x63, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, ENABLE_WRITE_MEMORY, IS_TWICE);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, WRITE_MEMORY_LOCATION, 0x01, IS_NORMAL);
        }
    }
    else if (type == WS_SET_FUNCTION_TEST) {
        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            sendUartDaliCommand(uartPort, nodeAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, START_FUNCTION_TEST, IS_TWICE);
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, START_FUNCTION_TEST, IS_TWICE);
        }
    }
    else if (type == WS_SET_DURATION_TEST) {
        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            sendUartDaliCommand(uartPort, nodeAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, START_DURATION_TEST, IS_TWICE);
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, START_DURATION_TEST, IS_TWICE);
        }
    }

    else if (type == WS_SET_STOP) {
        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            sendUartDaliCommand(uartPort, nodeAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, STOP_TEST, IS_TWICE);
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, STOP_TEST, IS_TWICE);
        }
    }
    else if (type == WS_SET_LOAD_NODES) {
        sendNodesFromDatabase(webServer, database);
    }
    else if (type == WS_SET_TEST  ) {
        QStringList webServerParts = value.split(" ");

        if (webServerParts.size() == 1) {                   // TODO DESHABILITADO
            disableAllTest(webServerParts, database);
        }
        else if (webServerParts.size() == 3) {              // FUNCTIONAL HABILITADO
            setFunctionalTest(webServerParts, database);
        }
        else if (webServerParts.size() == 4) {              // DURATION HABILITADO
            setDurationTest(webServerParts, database);
        }
        else if (webServerParts.size() == 6) {              // TODO HABILITADO
            setAllTest(webServerParts, database);
        }
    }
    else if (type == WS_SET_UPDATE_FILE) {
        qDebug() << "UPDATE FILE";
    }
    else if (type == WS_GET_LOGS) {
        qDebug() << "GETTING LOGS " << value;
    }
    else if (type == WS_GET_NODE_INFO) {
        sendNodeInfo(webServer, value);
    }
    else if (type == WS_SET_CLOSE_CONTROL) {
        isOpenNodeControl = false;
    }
    else if (type == WS_SET_READ_ID_CODE) {
        QStringList webServerParts = value.split("ñ");
        QString deviceID = webServerParts[1];
        sendWriteIDCodeFrame(uartPort, deviceID);
    }

    if (type != WS_SET_START_ACTION && type != WS_SET_DELETE_DEVICE && type != WS_SET_ADD_GROUP && type != WS_SET_DEL_GROUP && type != WS_SET_NEW_COMMISSION_ITERATION) {
        pollingTimer.start(POLLING_TIMER_MS);
    }
}

void sendLoginInfo(WebServer* webServer, uint8_t loginInfo)
{
    QString message = QString(WS_SEND_LOGIN_INFO) + "@" + QString::number(loginInfo);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendInterfaceInfo(WebServer* webServer, QString info)
{
    QString message = QString(WS_SEND_INTERFACES_INFO) + "@" + info;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendDateTimeInfo(WebServer* webServer, QString info)
{
    QString message = QString(WS_SEND_DATE_TIME_INFO) + "@" + info;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendIPConfigInfo(WebServer* webServer, bool ipConfigInfo)
{
    QString message = QString(WS_SEND_IPCONFIG_INFO) + "@" + QString::number(ipConfigInfo);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmStartCommission(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_START_COMMISSION) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmAddingDevice(WebServer* webServer)
{
    /*
    QString message = QString(WS_SEND_CONFIRM_ADDING_DEVICE) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
    */
}

void sendStartAddingDevices(WebServer* webServer)
{
    QString message = QString(WS_SEND_START_ADDING_DEVICES) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendScannedDevices(QByteArray data, WebServer* webServer)
{
    uint8_t uuid[16], emptyUUID[16] = {0};
    uint16_t reportAddress;

    for (uint8_t i = 0; i < 16; i++) { uuid[i] = (unsigned char)data[i + 5]; }
    reportAddress = ((unsigned char)data[3] << 8) + (unsigned char)data[4];

    commissionData.numberOfNodesScanned++;
    commissionData.isRelayNode = true;

    for (uint8_t i = 0; i < 20; i++) {
        if (memcmp(scannedUUID[i].UUID, emptyUUID, sizeof(emptyUUID)) == 0) {
            memcpy(scannedUUID[i].UUID, uuid, sizeof(uuid));
            scannedUUID[i].nodeAddressReport = reportAddress;
            break;
        }
    }
/*
    for (uint8_t i = 0; i < (MAX_SUBNET * MAX_NODES_SUBNET); i++) {
        if (scannedUUID[i].UUID == nullptr) {
            scannedUUID[i].UUID = new uint8_t[16];
            memcpy(scannedUUID[i].UUID, uuid, sizeof(uuid));
            scannedUUID[i].nodeAddressReport = reportAddress;
            commissionData.numberOfNodesScanned++;
            break;
        }
        else {
            if (!memcmp(scannedUUID[i].UUID, uuid, sizeof(uuid))) { break; }
        }
    }
*/
    QString value = "";
    for (uint8_t i = 0; i < 16 ; i++) { value += QString::asprintf("%02X", uuid[i]); }
    QString message = QString(WS_SEND_SCANNED_DEVICES) + "@" + value;

    qDebug() << "NODE SCANNED: " << value <<  " - REPORT ADDRESS: " << reportAddress;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendAddedDevices(QByteArray data, WebServer* webServer, Database* database)
{
    //uint8_t nodeUUID[16];
    //uint8_t netAddress[2];

    uint16_t nodeAddress = ((unsigned char)data[3] << 8) + (unsigned char)data[4];
    //for (uint8_t i = 0; i < 16; i++) { nodeUUID[i] = (unsigned char)data[5 + i]; }

    //setFirstAddressAvailable(nodeAddress, nodeUUID, database, netAddress);

    qDebug() << "UART FRAME RECEIVED: ADDED DEVICE " << nodeAddress;

    //QString message = QString(WS_SEND_ADDED_DEVICES) + "@" + QString::number(netAddress[0] * 64 + netAddress[1] + 1);

    //if (webServer != nullptr) { webServer->sendData(message); }
}

void sendDeviceError(QByteArray data, UartPort* uartPort, WebServer* webServer)
{
    uint8_t nodeUUID[16];

    commissionData.numberOfNodesAdded++;
    numberOfIterations++;

    QString  value = "";
    for (uint8_t i = 0; i < 16; i++) {
        nodeUUID[15 - i] = (unsigned char)data[3 + i];
        value += QString::asprintf("%02X", nodeUUID[i]);
    }

    for (uint8_t i = 0; i < 20; i++) {
        if (memcmp(scannedUUID[i].UUID, nodeUUID, sizeof(scannedUUID[i].UUID)) == 0) {
            qDebug() << "BORRADO EN DEVICE ERROR";
            memset(scannedUUID[i].UUID, 0, sizeof(scannedUUID[i].UUID));
            scannedUUID[i].nodeAddressReport = 0;
            break;
        }
    }

    delay(5000);

    uint8_t emptyUUID[16] = {0};
    for (uint8_t l = 0; l < 20; l++) {
        if (commissionData.numberOfNodesScanned == commissionData.numberOfNodesAdded) {
            commissionData.numberOfNodesScanned = 0;
            commissionData.numberOfNodesAdded = 0;
            sendUartNewIteration(uartPort);
            newIterationTimer.start(NEW_ITERATION_TIMER_MS);
            break;
        }
        if (memcmp(scannedUUID[l].UUID, emptyUUID, sizeof(emptyUUID)) != 0) {
            qDebug() << "ADDING NEW NODE UUID" << QString("0x%1").arg(scannedUUID[l].UUID[0], 2, 16, QChar('0')).toUpper();
            sendUartAddDevice(uartPort, scannedUUID[l]);
            confirmAddDeviceTimer.start(CONFIRM_ADD_DEVICE_TIMER_MS);
            break;
        }
    }

    QString message = QString(WS_SEND_DEVICE_ERROR) + "@" + "";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendNodesFromDatabase(WebServer* webServer, Database* database)
{
    QList<uint16_t> nodeNetAddressList = database->getConfiguredNodes();

    for (uint16_t nodeNetAddress : nodeNetAddressList) {
        QString message = QString(WS_SEND_ADDED_DEVICES) + "@" + QString::number(nodeNetAddress);
        if (webServer != nullptr) { webServer->sendData(message); }
        delay(WEBSERVER_SEND_TIME_MS);
    }
}

void sendNodeInfo(WebServer* webServer, QString nodeNetAddress)
{
    QString controlGearStatus, emergencyMode, emergencyFailureStatus, actualLvl, communicationFailure, deviceType;
    controlGearStatus = QString::number(meshDevice[(nodeNetAddress.toUInt() - 1) / 64][(nodeNetAddress.toUInt() - 1) % 64].getControlGearStatus());
    emergencyMode = QString::number(meshDevice[(nodeNetAddress.toUInt() - 1) / 64][(nodeNetAddress.toUInt() - 1) % 64].getEmergencyMode());
    emergencyFailureStatus = QString::number(meshDevice[(nodeNetAddress.toUInt() - 1) / 64][(nodeNetAddress.toUInt() - 1) % 64].getEmergencyFailureStatus());
    actualLvl = QString::number(meshDevice[(nodeNetAddress.toUInt() - 1) / 64][(nodeNetAddress.toUInt() - 1) % 64].getActualLvl());
    communicationFailure = QString::number(meshDevice[(nodeNetAddress.toUInt() - 1) / 64][(nodeNetAddress.toUInt() - 1) % 64].getComunicationFailure());
    deviceType = QString::number(meshDevice[(nodeNetAddress.toUInt() - 1) / 64][(nodeNetAddress.toUInt() - 1) % 64].getDeviceType());

    lastNetAddressClicked = nodeNetAddress.toUInt();
    isOpenNodeControl = true;

    QString message = QString(WS_SEND_NODE_INFO) + "@" + controlGearStatus + "." + emergencyMode + "." + emergencyFailureStatus + "." + actualLvl + "." + communicationFailure + "." + deviceType;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendEndNodeConfiguration(WebServer* webServer)
{
    QString value = "";
    QString message = QString(WS_SEND_END_NODE_CONFIG) + "@" + value;
    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendEndAutoCommission(WebServer* webServer)
{
    QString message = QString(WS_SEND_END_AUTO_COMMISSION) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendFactoryIDWrote(WebServer* webServer)
{
    QString message = QString(WS_SEND_FACTORY_ID_WROTE) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendDaliTested(WebServer* webServer)
{
    QString message = QString(WS_SEND_DALI_TESTED) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendRecordedDevice(WebServer* webServer)
{
    QString message = QString(WS_SEND_RECORDED_DEVICE) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}
