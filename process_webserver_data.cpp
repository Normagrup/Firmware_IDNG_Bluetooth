#include "process_webserver_data.h"
#include "process_uart_data.h"
#include "global_variables.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "time_functions.h"
#include "file_handler.h"
#include "log.h"
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
        scannedDevicesMessages.clear();
        sendUartScannedDevices(uartPort);
    }
    else if (type == WS_SET_STORED_SCANNED_DEVICES) {
        sendStoredScannedDevices(webServer);
    }
    else if (type == WS_SET_LINE_SCAN) {
        if(isCommissionInProgress(webServer)) { return; }
        requestMicroDatabase(uartPort);
        
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
            //if (uuidScanned.UUID != nullptr) { sendUartAddDevice(uartPort, uuidScanned); sendLogCommissionEntry(webServer, "Start adding node " + getUUIDAsString(uuidScanned)); }
        }
        else {
            qDebug() << "START COMMISSION";
            commissionData.numberOfNodesScanned = 0;
            commissionData.numberOfNodesAdded = 0;
            scannedDevicesMessages.clear();
            sendUartStartCommission(uartPort);
            delay(300);
            sendLogCommissionEntry(webServer, "Scanning devices...");
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
    else if (type == WS_SET_STOP_ACTION) {
        forceStopCommissioning = true;
        qDebug() << "Mensaje de detención de COMMISSIONING recibido.";
    }
    else if (type == WS_SET_DELETE_DEVICE) {
        if(isCommissionInProgress(webServer)) { return; }

        uint16_t nodeNetAddress = getNodeNetAddress(value);

        if(nodeNetAddress == 0xFFFF)
        {
            sendUartDelDevice(uartPort, nodeNetAddress);

            // Eliminar nodos de la estructura interna
            // TODO: Mover a la confirmación del micro
            for(int i = 0; i < MAX_SUBNET; i++){
                for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                    if(meshDevice[i][j].getIsConfigured()) {
                        uint16_t nodeNetAddr = i * 64 + j + 1;
                        insertDevToLog(nodeNetAddr, database, LOG_DEVICE_REMOVED);
                        meshDevice[i][j].deleteDevice();
                    }
                }
            }
            database->deleteAllNodes();
        }
        else
        {
            uint16_t nodeAddress = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            printf(" Net Address: %04X - RealAddress: %04X\n", nodeNetAddress, nodeAddress);

            sendUartDelDevice(uartPort, nodeAddress);

            // Device to delete added to log
            insertDevToLog(nodeNetAddress, database, LOG_DEVICE_REMOVED);

            // Eliminar el nodo de la estructura interna
            // TODO: Mover a la confirmación del micro
            meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].deleteDevice();
            database->deleteNode(nodeAddress);
        }
    }
    else if (type == WS_SET_ADD_DEVICE) {
        if (isCommissionInProgress(webServer)) { return; }

        // Extraer el índice del UUID correspondiente al nodo que queremos añadir
        int uuidIndex = getUUIDIndexOfScanned(value);

        if (uuidIndex == -1) { return; }

        isManualAddingDevice = true;

        // Guardar backup de la lista original
        memcpy(scannedUUIDBackup, scannedUUID, sizeof(scannedUUID));

        // Limpiar la lista original
        memset(scannedUUID, 0, sizeof(scannedUUID));

        // Copiar el UUID y el nodeAddressReport en la lista nueva
        memcpy(scannedUUID[0].UUID, scannedUUIDBackup[uuidIndex].UUID, sizeof(scannedUUIDBackup[0].UUID));
        scannedUUID[0].nodeAddressReport = scannedUUIDBackup[uuidIndex].nodeAddressReport;

        // Limpiar de la lista original el elemento correspondiente al nodo que queremos añadir
        for(int i = uuidIndex; i < 20; i++) {
            if(i != 19) {
                memcpy(scannedUUIDBackup[i].UUID, scannedUUIDBackup[i+1].UUID, sizeof(scannedUUIDBackup[i].UUID));
                scannedUUIDBackup[i].nodeAddressReport = scannedUUIDBackup[i+1].nodeAddressReport;
            }
            else {
                memset(scannedUUIDBackup[i].UUID, 0, sizeof(scannedUUIDBackup[i].UUID));
                scannedUUIDBackup[i].nodeAddressReport = 0;
            }
        }

        // Eliminar la entrada del dispositivo que añadimos de la lista de dispositivos escaneados que se muestra en el webserver
        scannedDevicesMessages.removeAt(uuidIndex);

        sendUartAddDevice(uartPort, scannedUUID[0]);
        sendLogCommissionEntry(webServer, "Start adding node " + getUUIDAsString(scannedUUID[0].UUID));
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
        sendUartAddGroupManual(uartPort, address);

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
        database->createGroup();
        sendGroups(webServer, database);
    }
    else if (type == WS_SET_DEL_A_GROUP) {
        database->removeGroup(value);
        uint16_t groupAddress = getOneGroupAddress(value);
        sendUartDelGroupForAllNodes(uartPort, groupAddress, database);
        sendGroups(webServer, database);
    }
    else if (type == WS_SET_EDIT_A_GROUP) {
        QStringList parts = value.split("#");
        QString address = parts[0];
        QString newName = parts[1];

        database->editGroup(address, newName);
        sendGroups(webServer, database);
    }
    else if (type == WS_GET_GROUP_NODES) {
        sendGroupNodes(webServer, value);
    }
    else if (type == WS_SET_MAX) {
        if(isCommissionInProgress(webServer)) { return; }

        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].setActualLvl(254); // 254 / 254 = 100%
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, RECALL_MAX_LVL, IS_NORMAL);
        }
        else {
            for(int i = 0; i < MAX_SUBNET; i++){
                for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                    Device& device = meshDevice[i][j];
                    if(device.isOnGroupSubAddress(nodeNetAddress))
                        device.setActualLvl(254); // 254 / 254 = 100%
                }
            }
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, RECALL_MAX_LVL, IS_NORMAL);
        }
    }
    else if (type == WS_SET_OFF) {
        if(isCommissionInProgress(webServer)) { return; }

        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].setActualLvl(0); // 0 / 254 = 0%
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, OFF, IS_NORMAL);
        }
        else {
            for(int i = 0; i < MAX_SUBNET; i++){
                for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                    Device& device = meshDevice[i][j];
                    if(device.isOnGroupSubAddress(nodeNetAddress))
                        device.setActualLvl(0); // 0 / 254 = 0%
                }
            }
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, OFF, IS_NORMAL);
        }
    }
    else if (type == WS_SET_MIN) {
        if(isCommissionInProgress(webServer)) { return; }

        uint16_t nodeNetAddress = value.toUInt();
        if (nodeNetAddress < 0xC000) {
            uint16_t nodeAddress =  meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
            meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].setActualLvl(3); // 3 / 254 = 1%
            sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, RECALL_MIN_LVL, IS_NORMAL);
        }
        else {
            for(int i = 0; i < MAX_SUBNET; i++){
                for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                    Device& device = meshDevice[i][j];
                    if(device.isOnGroupSubAddress(nodeNetAddress))
                        device.setActualLvl(3); // 3 / 254 = 1%
                }
            }
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

        if(values[0] < 0xC000) {
            meshDevice[(values[0] - 1) / 64][(values[0] - 1) % 64].setActualLvl(values[1]);
        }
        else {
            for(int i = 0; i < MAX_SUBNET; i++){
                for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                    Device& device = meshDevice[i][j];
                    if(device.isOnGroupSubAddress(values[0]))
                        device.setActualLvl(values[1]);
                }
            }
        }

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
            logTestRequest(database, nodeNetAddress, false, "FUNCTIONAL");
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, START_FUNCTION_TEST, IS_TWICE);
            logTestRequest(database, nodeNetAddress, true, "FUNCTIONAL");
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
            logTestRequest(database, nodeNetAddress, false, "DURATION");
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, START_DURATION_TEST, IS_TWICE);
            logTestRequest(database, nodeNetAddress, true, "DURATION");
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
            logTestRequest(database, nodeNetAddress, false, "STOP");
        }
        else {
            sendUartDaliCommand(uartPort, nodeNetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
            sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, STOP_TEST, IS_TWICE);
            logTestRequest(database, nodeNetAddress, true, "STOP");
        }
    }
    else if (type == WS_SET_LOAD_NODES) {
        sendNodesFromDatabase(webServer, database);
    }
    else if (type == WS_SET_IS_COMMISSION_IN_PROGRESS) {
        sendIsCommissionInProgress(webServer);
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
        QStringList webServerParts = value.split(" ");
        QString reportType = webServerParts[0];
        QString startDate = webServerParts[1];
        QString endDate = webServerParts[2];
        QString downloadPath;

        downloadPath = exportLogToCSV(database, reportType, startDate, endDate);
        QStringList ConfigInfo = database -> getInterfaceParameters();
        QString serverIP = ConfigInfo.first();
        QString fileUrl = "http://" + serverIP + "/logs/" + downloadPath;
        sendLogFile(webServer, fileUrl);
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
        clearSystemData(database, uartPort);
    }
    else if (type == WS_GET_POWER_ON_LEVEL) {
        sendGroupsWithPOL(webServer, database, value);
    }
    else if (type == WS_SET_POWER_ON_LEVEL) {
        if(isCommissionInProgress(webServer)) { return; }

        QStringList parts = value.split("_");
        uint16_t groupAddress = parts[0].toUShort(nullptr, 16);
        uint8_t powerOnLevel = static_cast<uint8_t>(parts[1].toUInt(nullptr, 10));

        sendUartDaliCommand(uartPort, groupAddress, DTR_0, powerOnLevel, IS_NORMAL);
        delay(SLEEP_DALI_TIME_MS);
        sendUartDaliCommand(uartPort, groupAddress, BROADCAST_ADDR, STORE_DTR_POWER_ON_LVL , IS_TWICE);
    }
    else if (type == WS_SET_SYNC_POL) {
        sendUartPOLForUpdate(uartPort, database);
    }
    else if (type == WS_SET_RELOAD_TREE) {
        buildTreeAndSendConfirm(webServer, database);
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

void sendLogCommissionEntry(WebServer* webServer, QString content)
{
    QString message = QString(WS_SEND_LOG_COMMISSION_ENTRY) + "@" + content;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmStartScan(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_START_SCAN) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmStartCommission(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_START_COMMISSION) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmAddingDevice(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_ADDING_DEVICE) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
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

    if(!isCommissioning)
        scannedDevicesMessages.append(message);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendStoredScannedDevices(WebServer* webServer)
{
    for(QString message : scannedDevicesMessages)
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

    sendLogCommissionEntry(webServer, "The device has been added.");

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
            sendLogCommissionEntry(webServer, "Start adding node " + getUUIDAsString(scannedUUID[l].UUID));
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

        QString message = QString(WS_SEND_ADDED_DEVICES) + "@" + QString::number(netAddress) + "_" + serialNumber + "_" + "false"; // el booleano indica que no se debe incrementar el contador del webserver
        if (webServer != nullptr) { webServer->sendData(message); }
        delay(WEBSERVER_SEND_TIME_MS);
    }
}

void sendIsCommissionInProgress(WebServer* webServer)
{
    QString message = QString(WS_SEND_IS_COMMISSION_IN_PROGRESS) + "@" + (isCommissioning ? "true" : "false");

    if (webServer != nullptr) { webServer->sendData(message); }
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
                uint16_t groupSubAddress[1];
                convertGroupSubStringToArray(groupAddress, groupSubAddress);
                if(device.isOnGroupSubAddress(groupSubAddress[0])) {
                    configDevs++;
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

void sendGroupNodes(WebServer* webServer, QString groupAddress) {
    QString messageIncludedNodeInit = QString(WS_SEND_GROUP_NODE_INCLUDED) + "@";
    QString messageNotIncludedNodeInit = QString(WS_SEND_GROUP_NODE_NOT_INCLUDED) + "@";
    QString message;

    for(int i = 0; i < MAX_SUBNET; i++){
        for(int j = 0; j < MAX_NODES_SUBNET; j++) {
            Device& device = meshDevice[i][j];
            if(device.getIsConfigured()) {
                uint16_t groupSubAddress[1];
                convertGroupSubStringToArray(groupAddress, groupSubAddress);

                if(device.isOnGroupSubAddress(groupSubAddress[0]))
                    message = messageIncludedNodeInit;
                else
                    message = messageNotIncludedNodeInit;

                message += QString::number(i * 64 + j + 1) + "_" + device.serialNumberString();

                if (webServer != nullptr) { webServer->sendData(message); }
            }
        }
    }
}

void sendGroupsWithPOL(WebServer* webServer, Database* database, QString value) {
    /**
    QStringList groupList = database->getPowerOnLevel(value.toInt());

    for (const QString& group : groupList) {
        QString message = QString(WS_SEND_GROUP_WITH_POL) + "@" + group;
        if (webServer != nullptr) { webServer->sendData(message); }
        delay(WEBSERVER_SEND_TIME_MS);
    }
    */

    QString groups = database->getPowerOnLevel(value.toInt()).join("#");

    QString message = QString(WS_SEND_GROUP_WITH_POL) + "@" + groups;

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

void sendLogFile(WebServer *webServer, QString fileDir)
{
    QString message = QString(WS_SEND_LOG_DATA) + "@" + fileDir;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void clearSystemData(Database* database,  UartPort* uartPort)
{
    // Borrado de la BBDD del embebido
    qDebug() << "[Embebido] Borrando datos de la BBDD...";
    database->clearAllData();

    // Borrado del modelo del embebido
    qDebug() << "[Embebido] Borrando datos de meshDevice...";
    for(int i = 0; i < MAX_SUBNET; i++)
        for(int j = 0; j < MAX_NODES_SUBNET; j++)
            meshDevice[i][j].deleteDevice();

    qDebug() << "[Embebido] Borrando datos de tests...";
    for(int i = 0; i < MAX_TEST; i++)
        tests[i].deleteTest();

    qDebug() << "[Embebido] Enviando CLEAR_ALL_DATA al micro por UART...";
    sendUartClearAllData(uartPort); 

}

void sendConfirmStartRemoveAllNodes(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_START_DEL_ALL_DEV) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmEndRemoveAllNodes(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_END_DEL_ALL_DEV) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}


void sendConfirmAddNodeToGroup(WebServer* webServer, uint16_t address, uint16_t deviceTypeGroupAddress, Database* database)
{
    // Añadir grupo en la BBDD
    database->setGroup(address, deviceTypeGroupAddress);

    // Añadir al modelo  
    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (meshDevice[i][j].getRealAddress() == address) {
                meshDevice[i][j].setGroupSubAddress(deviceTypeGroupAddress);
                break;
            }
        }
    }

    QString message = QString(WS_SEND_CONFIRM_ADD_NODE_TO_GROUP) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmPowerOnLevel(WebServer* webServer, uint8_t powerOnLevel, uint16_t groupAddress, Database* database)
{
    QString groupAddressString = QString("%1").arg(groupAddress, 4, 16, QLatin1Char('0')).toUpper();

    database->setPowerOnLevel(groupAddressString, powerOnLevel);

    QString message = QString(WS_SEND_CONFIRM_POWER_ON_LEVEL) + "@" + groupAddressString + "_" + QString::number(powerOnLevel);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void buildTreeAndSendConfirm(WebServer* webServer, Database* database)
{
    database->readNodesForTree();
    buildJsonTree();

    QString message = QString(WS_SEND_CONFIRM_SHOW_TREE) + "@" + "";

    if (webServer != nullptr) { webServer->sendData(message); }
}
