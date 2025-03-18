#include "process_webserver_data.h"
#include "process_uart_data.h"
#include "global_variables.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "time_functions.h"
#include "file_handler.h"
#include <QThread>

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
        if(isCommissionInProgress(webServer)) { return; }

        sendUartScannedDevices(uartPort);
    }
    else if (type == WS_GET_IP_CONFIG) {
        QStringList messages = database->getInterfaceParameters();
        QString message = messages.join(" ");
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
        if(isCommissionInProgress(webServer)) { return; }

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

            isCommissioning = true;  // Lock server from accepting new commands
            sendUartStartCommission(uartPort);

            sendConfirmStartCommission(webServer);
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
        if(isCommissionInProgress(webServer)) { return; }

        //uint16_t nodeNetAddress = getNodeNetAddress(value);
        //uint16_t  nodeAddress = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
        //meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].deleteDevice();
        //database->deleteNode(nodeAddress);
        //sendUartDelDevice(uartPort, 0xFFFF);
        uint16_t nodeNetAddress = getNodeNetAddress(value);
        uint16_t nodeAddress = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
        printf(" Intentando eliminar nodo...\n");
        printf(" Net Address: %04X\n", nodeNetAddress);
        printf(" Dirección obtenida de meshDevice: %04X\n", nodeAddress);

        // Notificar al microcontrolador maestro
        sendUartDelDevice(uartPort, nodeAddress);

        // Eliminar el nodo de la estructura interna
        meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].deleteDevice();

        // Eliminar el nodo de la base de datos
        database->deleteNode(nodeAddress);



        printf(" Nodo eliminado correctamente: %04X\n", nodeAddress);
    }
    else if (type == WS_SET_DELETE_ALL_DEVICES) {
        if(isCommissionInProgress(webServer)) { return; }

        QList<uint16_t> nodeNetAddressList = database->getConfiguredNodes();

        for (uint16_t nodeNetAddress : nodeNetAddressList) {
            uint16_t nodeAddress = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();

            qDebug() << "Eliminando Nodo (NodeNetAddress:" << nodeNetAddress << "--- NodeAddress:" << nodeAddress << ")";

            // Notificar al microcontrolador maestro
            sendUartDelDevice(uartPort, nodeAddress);

            // Esperar confirmación de eliminación verificando la base de datos
            bool eliminado = false;
            int intentos = 0;

            while (!eliminado && intentos < 3) {  // Intentar hasta 3 veces
                QThread::msleep(1000);  // Esperar 1 segundo para dar tiempo a la eliminación

                // Comprobar si el nodo sigue en la base de datos
                eliminado = !database->isNodeInDatabase(nodeAddress);
                intentos++;
            }

            if (eliminado) {
                qDebug() << "Nodo " << nodeAddress << " eliminado correctamente.";
                meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].deleteDevice();
            } else {
                qDebug() << "Error: Nodo " << nodeAddress << " no respondió a la eliminación.";
            }
        }

        // Eliminar todos los nodos de la base de datos
        database->deleteAllNodes();

        qDebug() << "Eliminación de nodos completada";
    }
    else if (type == WS_SET_ADD_GROUP) {
        if(isCommissionInProgress(webServer)) { return; }

        QStringList parts0 = value.split(" - "); // "Node 1 - [12.34.56.78] C010" -> "Node 1", "[12.34.56.78] C010"
        QStringList parts1 = parts0[1].split("]"); // "[12.34.56.78] C010" -> "[12.34.56.78", " C010"
        QString parsedValue = parts0[0] + parts1[1]; // "Node 1 C010"

        uint16_t* tmpAddress = getGroupAddress(parsedValue);

        uint16_t* address = new uint16_t[3];
        address[0] = tmpAddress[0];
        address[1] = tmpAddress[1];
        address[2] = 0x0000;

        timerGroupAddress[0] = address[0];
        timerGroupAddress[1] = address[1];
        timerGroupAddress[2] = 0x0000;
        groupDataConfiguration.configSecondGroup = false;
        qDebug() << "GROUP ADD";
        sendUartAddGroup(uartPort, address);
    }
    else if (type == WS_SET_DEL_GROUP) {
        if(isCommissionInProgress(webServer)) { return; }

        QStringList parts0 = value.split(" - "); // "Node 1 - [12.34.56.78] C010" -> "Node 1", "[12.34.56.78] C010"
        QStringList parts1 = parts0[1].split("]"); // "[12.34.56.78] C010" -> "[12.34.56.78", " C010"
        QString parsedValue = parts0[0] + parts1[1]; // "Node 1 C010"

        uint16_t* address = getGroupAddress(parsedValue);
        qDebug() << "GROUP DEL";
        sendUartDelGroup(uartPort, address, database);
    }
    else if (type == WS_SET_ADD_A_GROUP) {
        createGroup(value, database);
        sendGroups(webServer, database);
    }
    else if (type == WS_SET_DEL_A_GROUP) {
        removeGroup(value, database);
        uint16_t groupAddress = getOneGroupAddress(value);
        sendUartDelGroupForAllNodes(uartPort, groupAddress, database);
        sendGroups(webServer, database);
    }
    else if (type == WS_SET_MAX) {
        if(isCommissionInProgress(webServer)) { return; }

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
        if(isCommissionInProgress(webServer)) { return; }

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
        if(isCommissionInProgress(webServer)) { return; }

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
        if(isCommissionInProgress(webServer)) { return; }

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
        if(isCommissionInProgress(webServer)) { return; }

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
        if(isCommissionInProgress(webServer)) { return; }

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
        if(isCommissionInProgress(webServer)) { return; }

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
        if(isCommissionInProgress(webServer)) { return; }

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
        if(isCommissionInProgress(webServer)) { return; }

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
        if(isCommissionInProgress(webServer)) { return; }

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
    else if (type == WS_SET_TEST) {
        if(isCommissionInProgress(webServer)) { return; }

        QStringList webServerParts = value.split(" ");
        setTests(webServerParts, database);
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
    else if (type == WS_GET_DEVICES_COUNT) {
        int count = 0;
        for(int i = 0; i < MAX_SUBNET; i++){
            for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                Device& device = meshDevice[i][j];
                if(device.getIsConfigured())
                    count++;
            }
        }
        sendDevicesCount(webServer, count);
    }
    else if (type == WS_GET_FAILURES_COUNT) {
        int count = 0;
        int lampFailCount = 0;
        int batFailCount = 0;
        int durFailCount = 0;
        int comFailCount = 0;

        for(int i = 0; i < MAX_SUBNET; i++){
            for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                Device& device = meshDevice[i][j];
                if(device.getIsConfigured()) {
                    count += device.getTotalFailures();
                    if(device.hasLampFailure()) { lampFailCount++; }
                    if(device.hasBatteryFailure()) { batFailCount++; }
                    if(device.hasBatteryDurationFailure()) { durFailCount++; }
                    if(device.hasCommunicationFailure()) { comFailCount++; }
                }
            }
        }
        sendFailuresCount(webServer, count, lampFailCount, batFailCount, durFailCount, comFailCount);
    }
    else if (type == WS_GET_IS_CONFIG) {
        int subnet = (value.toInt() - 1) / MAX_NODES_SUBNET;
        int id = (value.toInt() - 1) % MAX_NODES_SUBNET;

        QString serialNumber = meshDevice[subnet][id].serialNumberString();
        bool isConfig = meshDevice[subnet][id].getIsConfigured();
        bool hasFailures = meshDevice[subnet][id].getTotalFailures() > 0;
        bool onOffStatus = meshDevice[subnet][id].getActualLvl() > 0;

        sendIsConfig(webServer, value, serialNumber, isConfig, hasFailures, onOffStatus);
    }
    else if (type == WS_GET_GROUPS) {
        sendGroups(webServer, database);
    }
    else if (type == WS_GET_GROUP_INFO) {
        sendGroupInfo(webServer, value);
    }
    else if (type == WS_GET_TEST) {
        sendTest(webServer, database, value);
    }
    else if (type == WS_SET_CLEAR_ALL_DATA) {
        if(isCommissionInProgress(webServer)) { return; }

        clearSystemData(database);
    }

    if (type != WS_SET_START_ACTION && type != WS_SET_DELETE_DEVICE && type != WS_SET_ADD_GROUP && type != WS_SET_DEL_GROUP && type != WS_SET_NEW_COMMISSION_ITERATION) {
        pollingTimer.start(POLLING_TIMER_MS);
    }
}

bool isCommissionInProgress(WebServer* webServer)
{
    if (isCommissioning) {
        QString message = QString(WS_SEND_ALERT_COMMISSION) + "@" + "Command blocked. Commissioning in progress.";
        if (webServer != nullptr) { webServer->sendData(message); }
        return true;
    }
    return false;
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
    QList<QPair<uint16_t, QString>> nodeNetAddressAndSNList = database->getConfiguredNodesAndSerialNumbers();

    for (const QPair<uint16_t, QString>& node : nodeNetAddressAndSNList) {
        uint16_t netAddress = node.first;
        QString serialNumber = node.second;

        QString message = QString(WS_SEND_ADDED_DEVICES) + "@" + QString::number(netAddress) + "_" + serialNumber;
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

void sendGroups(WebServer* webServer, Database* database) {
    QList<QPair<QString, QString>> groupList = database->getGroups();

    for (const QPair<QString, QString>& group : groupList) {
        QString groupAddress = group.first;
        QString groupName = group.second;

        QString message = QString(WS_SEND_GROUP) + "@" + groupAddress + "_" + groupName;
        if (webServer != nullptr) { webServer->sendData(message); }
        delay(WEBSERVER_SEND_TIME_MS);
    }
}

void sendGroupInfo(WebServer* webServer, QString groupAddress)
{
    int lampFailCount = 0, batFailCount = 0, durFailCount = 0, comFailCount = 0, emerModeCount = 0;
    int totalLvl = 0, configDevs = 0;

    for(int i = 0; i < MAX_SUBNET; i++){
        for(int j = 0; j < MAX_NODES_SUBNET; j++) {
            Device& device = meshDevice[i][j];
            if(device.getIsConfigured()) {
                configDevs++;
                uint16_t groupSubAddress[1];
                convertGroupSubStringToArray(groupAddress, groupSubAddress);
                if(device.isOnGroupSubAddress(groupSubAddress[0])) {
                    if(device.hasLampFailure()) { lampFailCount++; }
                    if(device.hasBatteryFailure()) { batFailCount++; }
                    if(device.hasBatteryDurationFailure()) { durFailCount++; }
                    if(device.hasCommunicationFailure()) { comFailCount++; }
                    if(device.isEmergencyModeActive()) { emerModeCount++; }
                    totalLvl += device.getActualLvl();
                }
            }
        }
    }

    int average = (configDevs == 0) ? 0 : totalLvl / configDevs;
    uint8_t averageLvl = average > 254 ? 254 : average; // Comprobación para asegurar que no produzca overflow

    // Conversión a cadena para pasar el mensaje
    QString lampFailCountS, batFailCountS, durFailCountS, comFailCountS, emerModeCountS, averageLvlS;
    lampFailCountS = QString::number(lampFailCount);
    batFailCountS = QString::number(batFailCount);
    durFailCountS = QString::number(durFailCount);
    comFailCountS = QString::number(comFailCount);
    emerModeCountS = QString::number(emerModeCount);
    averageLvlS = QString::number(averageLvl);

    QString message = QString(WS_SEND_GROUP_INFO) + "@" + lampFailCountS + "." + emerModeCountS + "." + batFailCountS + "." + durFailCountS + "." + averageLvlS + "." + comFailCountS;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendTest(WebServer* webServer, Database* database, QString groupAddress)
{
    QString testData = database->getTests(groupAddress);

    if(testData.isEmpty()) { return; }

    QString message = QString(WS_SEND_TEST) + "@" + testData;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendDevicesCount(WebServer* webServer, int counter) {
    QString message = QString(WS_SEND_DEVICES_COUNTER) + "@" + QString::number(counter);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendFailuresCount(WebServer* webServer, int counter, int lampFailCounter, int batFailCounter, int durFailCounter, int comFailCounter) {
    QString message = QString(WS_SEND_FAILURES_COUNTER) + "@" + QString::number(counter)+ "." + QString::number(lampFailCounter)+ "." + QString::number(batFailCounter)+ "."+ QString::number(durFailCounter)+ "." + QString::number(comFailCounter);

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

void sendIsConfig(WebServer* webServer, QString device, QString serialNumber, bool isConfig, bool hasFailures, bool onOffStatus) {
    QString message = QString(WS_SEND_IS_CONFIG) + "@" + device + "_" + serialNumber + "_" + (isConfig ? "true" : "false") + "_" + (hasFailures ? "true" : "false") + "_" + (onOffStatus ? "on" : "off");

    if (webServer != nullptr) { webServer->sendData(message); }
}

void clearSystemData(Database* database)
{
    // Borrado de la BBDD del embebido
    database->clearAllData();

    // Borrado del modelo del embebido
    for(int i = 0; i < MAX_SUBNET; i++)
        for(int j = 0; j < MAX_NODES_SUBNET; j++)
            meshDevice[i][j].deleteDevice();

    for(int i = 0; i < MAX_TEST; i++)
        tests[i].deleteTest();

    // Borrado del micro
    // TODO
}
