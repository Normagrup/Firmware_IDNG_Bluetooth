#include "process_webserver_data.h"
#include "process_uart_data.h"
#include "global_variables.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "time_functions.h"
#include "file_handler.h"
#include "log.h"
#include <QThread>

void sendIdentify(UartPort* uartPort, uint16_t nodeNetAddress)
{
    if (nodeNetAddress < 0xC000) {
        uint16_t nodeAddress = meshDevice[(nodeNetAddress - 1) / 64]
                                         [(nodeNetAddress - 1) % 64]
                                             .getRealAddress();

        sendUartDaliCommand(uartPort, nodeAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
        delay(SLEEP_DALI_TIME_MS);
        sendUartDaliCommand(uartPort, nodeAddress, BROADCAST_ADDR, 0xF0, IS_TWICE);
    } else {
        sendUartDaliCommand(uartPort, nodeNetAddress, BROADCAST_ADDR, IDENTIFY_DEVICE, IS_TWICE);
    }
}

void processWebServerData(QString data, WebServer* webServer, UartPort* uartPort, Database* database)
{
    QStringList dataParts = data.split("@");
    QString type = dataParts[0].trimmed();
    QString value = dataParts[1].trimmed();

    qDebug() << "WEBSERVER FRAME RECEIVED: " << type;

    if(embeddedState == RECOVERING_MICRO) {
        sendRecoveringMicro(webServer);
        return;
    }

    pollingTimer.stop();

    if (type == WS_SET_LOG_IN) {
        uint8_t loginInfo = database->verifyLoginParameters(value);
        sendLoginInfo(webServer, loginInfo);
    }
    else if (type == WS_SET_REBOOT_DEVICE) {
        sendUartMicroReboot(uartPort);
        rebootDevice();
    }
    else if (type == WS_ASK_STATE_TO_EMBEDDED) {
        // -------- Por si se cambia de página, que se detenga el identify que se esté haciendo
        if (identifyTimer.isActive()) { identifyTimer.stop(); cleanCdbTimer.start(TIME_TO_CLEAN_CDB); }
        identifyNodeNetAddress = 0;
        identifyIteration = 0;
        // ------------------------------------------------------------------------------------

        QString st;

        switch (embeddedState) {
            case FREE: st = "FREE"; break;
            case SCAN: st = "SCAN"; break;
            case COMMISSION: st = "COMMISSION"; break;
            case ADD_MANUAL: st = "ADD_MANUAL"; break;
            case REPLACE: st = "REPLACE"; break;
            case DEL_DEV_BC: st = "DEL_DEV_BC"; break;
            case DEL_DEV: st = "DEL_DEV"; break;
            case SETTER_RELAY: st = "SETTER_RELAY"; break;
            case ADD_NODE_TO_GROUP: st = "ADD_NODE_TO_GROUP"; break;
            case DEL_NODE_FROM_GROUP: st = "DEL_NODE_FROM_GROUP"; break;
            case DEL_COMPLETE_GROUP: st = "DEL_COMPLETE_GROUP"; break;
            case CLEAR_ALL: st = "CLEAR_ALL"; break;
            case SYNC_POL: st = "SYNC_POL"; break;
            case SCAN_BY_NODE: st = "SCAN_BY_NODE"; break;
            case LINE_SCAN: st = "LINE_SCAN"; break;
            case APPLY_AUTOASSIGNMENT: st = "APPLY_AUTOASSIGNMENT"; break;
            case GROUP_AUTOASSIGNMENT: st = "GROUP_AUTOASSIGNMENT"; break;
        }

        QString message = QString(WS_ASK_STATE_TO_EMBEDDED) + "@" + value + "#" + st;

        if (webServer != nullptr) { webServer->sendData(message); }
    }
    else if (type == WS_SET_SCANNED_DEVICES) {
        embeddedState = SCAN;
        cleanCdbTimer.stop();
        // confirmación de inicio en respuesta de UART

        memset(scannedUUID, 0, sizeof(scannedUUID));
        sendUartScannedDevices(uartPort);

        delay(10000);

        sendConfirmEndScan(webServer);
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        embeddedState = FREE;
    }
    else if (type == WS_SET_SCAN_FROM_NODE) {
        embeddedState = SCAN_BY_NODE;
        cleanCdbTimer.stop();
        sendConfirmStartScan(webServer); // confirmación de inicio duplicada por si acaso en respuesta de UART

        memset(scannedUUID, 0, sizeof(scannedUUID));

        uint16_t nodeNetAddress = getNodeNetAddress(value);
        uint16_t nodeRealAddress = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();

        qDebug() << "Iniciando escaneo desde nodo realAddress:" << nodeRealAddress;

        sendUartInyectNode(uartPort, nodeRealAddress, database);
        while(messageState == PENDING) {}

        if(messageState == RECEIVED) {
            sendUartScanFromNode(uartPort, nodeRealAddress);
            while(messageState == PENDING) {}
            delay(10300);
        }

        sendUartClearInyectedNodes(uartPort, false, database);
        while(messageState == PENDING) {}

        sendConfirmEndScan(webServer);
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        embeddedState = FREE;
    }

    else if (type == WS_SET_STORED_SCANNED_DEVICES) {
        sendStoredScannedDevices(webServer);
    }
    else if (type == WS_GET_IP_CONFIG) {
        QStringList messages = database->getInterfaceParameters();
        QString message = messages.join(" ");
        sendInterfaceInfo(webServer, message);
    }
    else if (type == WS_SET_IP_CONFIG) {
        QStringList webServerParts = value.split(" ");
        QString password = webServerParts[webServerParts.size() - 1];

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
        embeddedState = COMMISSION;
        cleanCdbTimer.stop();
        // confirmación de inicio en respuesta de UART

        qDebug() << "START COMMISSION";

        database->clearPartialUnassignedNodes();

        numberOfIterations = 0;
        doneIterations = 0;
        for(int i = 0; i < MAX_SUBNET; i++){
            for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                if(meshDevice[i][j].getIsConfigured())
                    numberOfIterations++;
            }
        }
        memset(scannedUUID, 0, sizeof(scannedUUID));

        commissionData.numberOfNodesScanned = 0;
        commissionData.numberOfNodesAdded = 0;
        sendUartStartCommission(uartPort, database->getNextUnicastAddress());
        delay(300);
        sendLogCommissionEntry(webServer, "Scanning devices...", "INFO");
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
        cleanCdbTimer.stop();

        database->clearPartialUnassignedNodes();

        uint16_t nodeNetAddress = getNodeNetAddress(value);

        if(nodeNetAddress == 0xFFFF)
        {
            embeddedState = DEL_DEV_BC;
            sendConfirmStartRemoveAllNodes(webServer);

            sendEstimatedTime(webServer, 30); // 30s de margen para que se borren todos los nodos

            uint16_t dummyAddress = 0xFFFF;
            sendUartDelDevice(uartPort, dummyAddress, true);
            while(messageState == PENDING) {}

            if(messageState == RECEIVED) { insertDevToLog(0xFFFF, database, LOG_DEVICE_REMOVED, "Device"); }

            database->deleteAllNodes();

            for (int i = 0; i < MAX_SUBNET; i++)
                for (int j = 0; j < MAX_NODES_SUBNET; j++)
                    meshDevice[i][j].deleteDevice();

            delay(30000);
            sendConfirmEndRemoveAllNodes(webServer);
            cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
            embeddedState = FREE;
        }
        else
        {
            embeddedState = DEL_DEV;
            sendConfirmStartRemoveOneNode(webServer);

            uint16_t nodeAddress = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();

            // Borrado del dispositivo elegido
            printf(" Net Address: %04X - RealAddress: %04X\n", nodeNetAddress, nodeAddress);

            sendUartInyectNode(uartPort, nodeAddress, database);
            while(messageState == PENDING) {}

            if(messageState == RECEIVED) {
                sendUartDelDevice(uartPort, nodeAddress, false);
                while(messageState == PENDING) {}
            }

            sendUartClearInyectedNodes(uartPort, false, database);
            while(messageState == PENDING) {}

            // Device to delete added to log
            insertDevToLog(nodeAddress, database, LOG_DEVICE_REMOVED, "Device");

            // Eliminar el nodo de la estructura interna
            meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].deleteDevice();
            database->deleteNode(nodeAddress);

            sendConfirmEndRemoveOneNode(webServer);
            cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
            embeddedState = FREE;
        }
    }
    else if (type == WS_SET_ADD_DEVICE) {
        embeddedState = ADD_MANUAL;
        cleanCdbTimer.stop();
        // no tiene confirmación de inicio, el webserver lo muestra automáticamente

        database->clearPartialUnassignedNodes();

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

        sendUartUpdateNextUnicast(uartPort, database->getNextUnicastAddress());
        delay(300);

        if (scannedUUID[0].nodeAddressReport != antennaRealAddress){
            sendUartInyectNode(uartPort, scannedUUID[0].nodeAddressReport, database);
            while(messageState == PENDING) {}

            if(messageState == RECEIVED) {
                sendUartSetRelay(uartPort, scannedUUID[0].nodeAddressReport, true);
                while(messageState == PENDING) {}
            }
        }

        sendUartAddDevice(uartPort, scannedUUID[0]);
        sendLogCommissionEntry(webServer, "Start adding node " + getUUIDAsString(scannedUUID[0].UUID), "INFO");
        confirmAddDeviceTimer.start(CONFIRM_ADD_DEVICE_TIMER_MS);
    }  
    else if (type == WS_SET_ADD_GROUP) {
        embeddedState = ADD_NODE_TO_GROUP;
        cleanCdbTimer.stop();
        // no tiene confirmación de inicio, el webserver lo muestra automáticamente

        bool added = false;

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

        sendUartInyectNode(uartPort, address[0], database);
        while(messageState == PENDING) {}

        if(messageState == RECEIVED) {
            sendUartAddGroupManual(uartPort, address);
            while(messageState == PENDING) {}

            if(messageState == RECEIVED) { added = true; }
        }

        sendUartClearInyectedNodes(uartPort, false, database);
        while(messageState == PENDING) {}

        sendConfirmAddNodeToGroup(webServer, address[0], address[1], added, database);
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        embeddedState = FREE;
    }
    else if (type == WS_SET_DEL_GROUP) {
        embeddedState = DEL_NODE_FROM_GROUP;
        cleanCdbTimer.stop();
        // no tiene confirmación de inicio, el webserver lo muestra automáticamente

        QStringList parts0 = value.split(" - "); // "Node 1 - [12.34.56.78] C010" -> "Node 1", "[12.34.56.78] C010"
        QStringList parts1 = parts0[1].split("]"); // "[12.34.56.78] C010" -> "[12.34.56.78", " C010"
        QString parsedValue = parts0[0] + parts1[1]; // "Node 1 C010"

        uint16_t* address = getGroupAddress(parsedValue);
        qDebug() << "GROUP DEL";

        sendUartInyectNode(uartPort, address[0], database);
        while(messageState == PENDING) {}

        if(messageState == RECEIVED) {
            sendUartDelGroup(uartPort, address, database);
            while(messageState == PENDING) {}
        }

        sendUartClearInyectedNodes(uartPort, false, database);
        while(messageState == PENDING) {}

        sendConfirmDelNodeFromGroup(webServer);
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        embeddedState = FREE;
    }
    else if (type == WS_SET_ADD_A_GROUP) {
        database->createGroup();
        sendGroups(webServer, database);
    }
    else if (type == WS_SET_DEL_A_GROUP) {
        embeddedState = DEL_COMPLETE_GROUP;
        cleanCdbTimer.stop();
        // no tiene confirmación de inicio, el webserver lo muestra automáticamente

        uint16_t groupAddress = getOneGroupAddress(value);

        int count = 0;
        for(int i = 0; i < MAX_SUBNET; i++){
            for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                if(meshDevice[i][j].getIsConfigured() && meshDevice[i][j].isOnSubList(groupAddress))
                    count++;
            }
        }
        sendEstimatedTime(webServer, 1 + count * 3); // 1 de base, 3 segundos por dispositivo

        database->removeGroup(value);
        sendUartDelGroupForAllNodes(uartPort, groupAddress, database);
        //sendGroups(webServer, database);

        sendConfirmDelGroup(webServer);
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        embeddedState = FREE;
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
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

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
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

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
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

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
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

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
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

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
    else if (type == WS_SET_RELAY_MODE) {
        embeddedState = SETTER_RELAY;
        cleanCdbTimer.stop();
        // no tiene confirmación de inicio, el webserver lo muestra automáticamente

        QStringList parts = value.split("_");
        uint16_t netAddress = parts[0].toUInt();
        bool enable = parts[1].toInt();

        uint16_t realAddress = meshDevice[(netAddress - 1) / 64][(netAddress - 1) % 64].getRealAddress();

        sendUartInyectNode(uartPort, realAddress, database);
        while(messageState == PENDING) {}

        if(messageState == RECEIVED) {
            sendUartSetRelay(uartPort, realAddress, enable);
            while(messageState == PENDING) {}
        }

        sendUartClearInyectedNodes(uartPort, false, database);
        while(messageState == PENDING) {}

        delay(1000);
        sendConfirmSetRelay(webServer);
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        embeddedState = FREE;
    }
    else if (type == WS_SET_IDENTIFY) {
        cleanCdbTimer.stop();

        uint16_t nodeNetAddress = value.toUInt();

        if (identifyTimer.isActive()) { identifyTimer.stop(); }

        identifyNodeNetAddress = nodeNetAddress;
        identifyIteration = 0;

        sendIdentify(uartPort, identifyNodeNetAddress);

        identifyTimer.start();

    }
    else if (type == WS_STOP_IDENTIFY) {
        if (identifyTimer.isActive()) { identifyTimer.stop(); cleanCdbTimer.start(TIME_TO_CLEAN_CDB); }
        identifyNodeNetAddress = 0;
        identifyIteration = 0;
    }
    else if (type == WS_SET_FACTORY_SETTINGS) {
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

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
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

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
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

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
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

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
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

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
    else if (type == WS_SET_TEST) {
        QStringList webServerParts = value.split(" ");
        setTests(webServerParts, database);
    }
    else if (type == WS_SET_UPDATE_FILE) {
        qDebug() << "UPDATE FILE";
    }
    else if (type == WS_GET_LOGS) {
        qDebug() << "GETTING LOGS " << value;
        QStringList webServerParts = value.split(" ");
        reportType = webServerParts[0];
        startDate = webServerParts[1];
        endDate = webServerParts[2];

        QDate startQDate = QDate::fromString(startDate, "yyyy-MM-dd");
        QDate endQDate = QDate::fromString(endDate, "yyyy-MM-dd");
        QDateTime startDT(startQDate, QTime(0, 0, 0));
        QDateTime endDT(endQDate, QTime(23, 59, 59));
        qint64 start = startDT.toSecsSinceEpoch();
        qint64 end = endDT.toSecsSinceEpoch();

        QList<QStringList> logs = database->getLogEventPaged(reportType, start, end, 1);
        transformEventCodes(&logs);

        sendLogData(webServer, logs);
    }
    else if (type == WS_GET_LOGS_PAGED) {
        int page = value.toInt();

        QDate startQDate = QDate::fromString(startDate, "yyyy-MM-dd");
        QDate endQDate = QDate::fromString(endDate, "yyyy-MM-dd");
        QDateTime startDT(startQDate, QTime(0, 0, 0));
        QDateTime endDT(endQDate, QTime(23, 59, 59));
        qint64 start = startDT.toSecsSinceEpoch();
        qint64 end = endDT.toSecsSinceEpoch();

        QList<QStringList> logs = database->getLogEventPaged(reportType, start, end, page);
        transformEventCodes(&logs);

        sendLogData(webServer, logs);
    }
    else if (type == WS_DOWNLOAD_LOGS) {
        qDebug() << "DOWNLOADING LOGS " << value;
        QStringList webServerParts = value.split(" ");
        QString reportType = webServerParts[0];
        QString startDate = webServerParts[1];
        QString endDate = webServerParts[2];

        QString fileName = exportLogToCSV(database, reportType, startDate, endDate);
        QStringList configInfo = database -> getInterfaceParameters();
        QString serverIP = configInfo.first();
        QString fileUrl = "http://" + serverIP + "/logs/" + fileName;
        sendLogFile(webServer, fileUrl);
    }
    else if (type == WS_GET_NODE_INFO) {
        sendNodeInfo(webServer, value);
    }
    else if (type == WS_SET_CLOSE_CONTROL) {
        isOpenNodeControl = false;
    }
    else if (type == WS_SET_READ_ID_CODE) {
        // no se pone embeddedState porque es una funcionalidad a parte (factory)
        cleanCdbTimer.stop();
        // no tiene confirmación de inicio, el webserver lo muestra automáticamente

        QString deviceID = value;
        qDebug() << "[ID_CODE] deviceID =" << deviceID;

        sendWriteIDCodeFrame(uartPort, deviceID);
        while(messageState == PENDING) {}
        sendFactoryIDWrote(webServer, messageState == RECEIVED);

        if(messageState == RECEIVED) {
            delay(1000);
            sendDaliTestForWriteID(uartPort, deviceID);
            while(messageState == PENDING) {}
            sendDaliTested(webServer, messageState == RECEIVED);

            if(messageState == RECEIVED) {
                delay(1000);
                sendEndRecordDevice(uartPort, deviceID);
                while(messageState == PENDING) {}
                sendRecordedDevice(webServer, messageState == RECEIVED);
            }
        }

        // confirmación en la respuesta al finalizar el escaneo
        // start del cleanCdbTimer en la respuesta al finalizar el escaneo
        // no se pone embeddedState porque es una funcionalidad a parte (factory)
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

        uint8_t emergencyMode = meshDevice[subnet][id].getEmergencyMode();
        bool isInEmergency = (emergencyMode >> 2) & 1;

        sendIsConfig(webServer, value, serialNumber, isConfig, hasFailures, onOffStatus, isInEmergency);
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
        embeddedState = CLEAR_ALL;
        cleanCdbTimer.stop();
        // no tiene confirmación de inicio, el webserver lo muestra automáticamente

        database->clearPartialUnassignedNodes();
        isClearingAllData = true;

        clearSystemData(webServer, database, uartPort);

        isClearingAllData = false;

        delay(30000);
        sendConfirmEndClearAllData(webServer);
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        embeddedState = FREE;
    }
    else if (type == WS_LINE_SCANNING) {
        embeddedState = LINE_SCAN;
        cleanCdbTimer.stop();
        sendConfirmStartLineScanning(webServer);

        database->clearPartialUnassignedNodes();

        uint16_t startAddr = value.split("_")[0].toUShort(nullptr, 10);
        uint16_t endAddr = value.split("_")[1].toUShort(nullptr, 10);

        sendUartStartLineScanning(uartPort);
        while(messageState == PENDING) {}

        // Si se ha recibido el mensaje de confirmación del micro, empieza
        if(messageState == RECEIVED) {
            for(uint16_t i = startAddr; i <= endAddr; i++) {
                if(forceStopLS1) { forceStopLS1 = false; break; }

                if (!database->isExistingNode(i)) {
                    sendLSInfo(webServer, i, 1);
                    sendUartLineScanning(uartPort, 1, i);
                    delay(5000);
                }
            }

            sendLSInfo(webServer, 0, 0);
            delay(2500);

            for(uint16_t j = 0; j < discovered_nodes_count; j++) {
                if(forceStopLS2) { forceStopLS2 = false; break; }

                if (database->isExistingNode(discovered_nodes[j])) {
                    sendLSInfo(webServer, discovered_nodes[j], 2);
                    sendUartLineScanning(uartPort, 2, discovered_nodes[j]);
                    delay(5000);
                }
            }

            delay(5000);
        }

        sendUartClearInyectedNodes(uartPort, false, database);
        while(messageState == PENDING) {}

        sendUartEndLineScanning(uartPort);
        while(messageState == PENDING) {}

        sendConfirmEndLineScanning(webServer);
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        embeddedState = FREE;
    }
    else if (type == WS_GET_POWER_ON_LEVEL) {
        sendGroupsWithPOL(webServer, database, value);
    }
    else if (type == WS_SET_POWER_ON_LEVEL) {
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);

        QStringList parts = value.split("_");
        uint16_t groupAddress = parts[0].toUShort(nullptr, 16);
        uint8_t powerOnLevel = static_cast<uint8_t>(parts[1].toUInt(nullptr, 10));

        sendUartDaliCommand(uartPort, groupAddress, DTR_0, powerOnLevel, IS_NORMAL);
        delay(SLEEP_DALI_TIME_MS);
        sendUartDaliCommand(uartPort, groupAddress, BROADCAST_ADDR, STORE_DTR_POWER_ON_LVL , IS_TWICE);
    }
    else if (type == WS_SET_SYNC_POL) {
        cleanCdbTimer.stop();
        embeddedState = SYNC_POL;
        // no tiene confirmación de inicio, el webserver lo muestra automáticamente

        sendUartPOLForUpdate(uartPort, database, webServer);

        sendConfirmEndSyncPOL(webServer);
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        embeddedState = FREE;
    }
    else if (type == WS_GET_MASTER_REAL_ADDRESS) {
        QString message = QString(WS_SEND_CONFIRM_M_ADDRESS_GET) + "@" + QString::number(antennaRealAddress);

        if (webServer != nullptr) { webServer->sendData(message); }
    }
    else if (type == WS_GET_INSTALL_KEY) {
        QString installKey = database->getInstallKey();

        QString message = QString(WS_SEND_INSTALL_KEY_GET) + "@" + (installKey.size() == 32 ? "16" : installKey);

        if (webServer != nullptr) { webServer->sendData(message); }
    }
    else if (type == WS_GET_FAILCOM_CYCLES) {
        sendFailComCycles(webServer);
    }
    else if (type == WS_SET_FAILCOM_CYCLES) {
        uint8_t cycles = value.toUInt();
        database->updateFailComCycles(cycles);
        failComCycles = cycles;
    }
    else if (type == WS_GET_ACTIVE_KEY_AND_FORCE) {
        uint8_t activeKey = database->getActiveKey();
        bool forceInstallKey = database->isForcingInstallKey();
        sendActiveKeyAndForcing(webServer, activeKey, forceInstallKey);
    }
    else if (type == WS_SET_ACTIVE_KEY) {
        uint8_t activeKey = value.toUInt();
        database->setActiveKey(activeKey);
        sendUartChangedActiveKey(uartPort, activeKey);
    }
    else if (type == WS_CHANGE_NODES) {
        database->clearPartialUnassignedNodes();

        QStringList positions = value.split("_");
        uint16_t position1 = positions[0].toUInt();
        uint16_t position2 = positions[1].toUInt();
        changePositions(database, position1, position2);
    }
    else if (type == WS_GET_LINE_SCANNED_NODES) {
        sendFoundNodes(webServer, scannedNodesCounter);
    }
    else if (type == WS_STOP_LS) {
        if(value == "1")
            forceStopLS1 = true;
        else if(value == "2")
            forceStopLS2 = true;
    }
    else if (type == WS_SET_MASTER_ADDR_AND_INSTALLKEY) {
        QStringList elems = value.split("_");
        QString antennaID = elems[0];
        QString installKey = elems[1];

        // Si cambia la antennaID
        if(antennaID != "") {
            // Para que la antena núm. 1 sea la address 31768 (0x7C18), la núm. 2 sea la address 31769 (0x7C19), etc. Hasta la núm. 1000, que será la 32767 (0x7FFF)
            int numValue = antennaID.toInt(nullptr, 10) + 31767;
            uint16_t newAntennaRealAddress = static_cast<uint16_t>(numValue);

            database->setMasterRealAddress(newAntennaRealAddress);
            // antennaRealAddress = newAntennaRealAddress;
        }

        // Si cambia la installKey
        if(installKey != "") {
            database->setInstallKey(installKey);
            database->clearAllData();
        }

        uint16_t mra = database->getMasterRealAddress();
        QString ik = database->getInstallKey();
        saveInstallKeyAndMasterAddress(getLocalDate(), getLocalTime(), ik, mra);

        delay(100);

        sendAntennaAddressAndInstallKey(uartPort, database);

        delay(400);

        rebootDevice();
    }
    else if (type == WS_REPLACE_NODES) {
        embeddedState = REPLACE;
        cleanCdbTimer.stop();
        sendConfirmStartReplace(webServer);

        database->clearPartialUnassignedNodes();
        isReplacingDevices = true;

        QStringList elems = value.split("_");
        replaceData.newNodeUUID = elems[0];
        replaceData.newNodeRealAddress = 0x0000;
        replaceData.oldNodeID = elems[1];
        replaceData.oldNodeRealAddress = 0x0000;

        addNodeForReplace(webServer, uartPort, database);
    }
    else if (type == WS_ADD_UNASSIGNED_NODE) {
        uint8_t result = database->addUnassignedNode(value);

        if(result == 3) // todo correcto
        {
            uint16_t unassignedNodesCount = database->getUnassignedNodesCount();
            uint16_t page;

            if(unassignedNodesCount == 0) { page = 1; }
            else { page = ((unassignedNodesCount - 1) / 16) + 1; }

            sendUnassignedNodesPaged(webServer, database, page);
        }
        else {
            sendAddUnassignedError(webServer, result);
        }
    }
    else if (type == WS_GET_UNASSIGNED_NODES_PAGED) {
        sendUnassignedNodesPaged(webServer, database, value.toInt());
    }
    else if (type == WS_AUTOASSIGNMENT) {
        if(database->doAutoAssignment())
            sendUnassignedNodesPaged(webServer, database, value.toInt());
    }
    else if (type == WS_APPLY_AUTOASSIGNMENT) {
        applyAutoAssignment(webServer, uartPort, database);
    }
    else if (type == WS_DEL_UNASSIGNED_NODE) {
        QStringList parts = value.split("_");
        QString serial = parts[0];
        uint16_t page = parts[1].toUInt();

        if(database->delUnassignedNode(serial))
        {
            sendUnassignedNodesPaged(webServer, database, page);
        }
    }
    else if (type == WS_GROUP_AUTOASSIGNMENT) {
        embeddedState = GROUP_AUTOASSIGNMENT;
        cleanCdbTimer.stop();
        sendConfirmStartGroupAutoAssignment(webServer);

        applyGroupAutoAssignment(webServer, uartPort, database);

        sendConfirmEndGroupAutoAssignment(webServer);
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        embeddedState = FREE;
    }
    else if (type == WS_UPDATE_UNASSIGNED) {
        database->clearUnassignedNodes();
        for(int n = 0; n < commissionedNodes.size(); n++) {
            database->deleteNode(meshDevice[commissionedNodes[n].i][commissionedNodes[n].j].getRealAddress());
            meshDevice[commissionedNodes[n].i][commissionedNodes[n].j].deleteDevice();

            database->addUnassignedNode(commissionedNodes[n].serial);
        }
        database->loadNodesFromDatabase();
        commissionedNodes.clear();
    }
    else if (type == WS_SWITCH_FORCE) {
        database->switchForcingInstallKey();
    }
    else if (type == WS_BLINK_ASSIGNED) {
        sendUartBlinkAssigned(uartPort);
    }
    else if (type == WS_BLINK_UNASSIGNED) {
        sendUartBlinkUnassigned(uartPort);
    }
    else if (type == WS_BLINK_STOP) {
        sendUartStopBlink(uartPort);
    }
    else if (type == WS_BLINK_UNASSIGNED_NODE) {
        sendUartBlinkUnassignedNode(uartPort, value);
    }
    else if (type == WS_CLEAR_RPL) {
        sendUartClearRpl(uartPort);
    }

    if (type != WS_SET_START_ACTION && type != WS_SET_ADD_GROUP && type != WS_SET_DEL_GROUP && type != WS_SET_NEW_COMMISSION_ITERATION) {
        pollingTimer.start(POLLING_TIMER_MS);
    }
}

void addNodeForReplace(WebServer* webServer, UartPort* uartPort, Database* database) {
    // PARTE 1 de 3: AÑADIR NODO NUEVO

    // Extraer el índice del UUID correspondiente al nodo que queremos añadir
    int uuidIndex = getUUIDIndexOfScanned(replaceData.newNodeUUID);

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

    sendUartUpdateNextUnicast(uartPort, database->getNextUnicastAddress());
    delay(300);

    if (scannedUUID[0].nodeAddressReport != antennaRealAddress) {
        sendUartInyectNode(uartPort, scannedUUID[0].nodeAddressReport, database);
        while(messageState == PENDING) {}

        if(messageState == RECEIVED) {
            sendUartSetRelay(uartPort, scannedUUID[0].nodeAddressReport, true);
            while(messageState == PENDING) {}
        }
    }

    sendUartAddDevice(uartPort, scannedUUID[0]);
    sendLogCommissionEntry(webServer, "Start adding node " + getUUIDAsString(scannedUUID[0].UUID), "INFO");
    confirmAddDeviceTimer.start(CONFIRM_ADD_DEVICE_TIMER_MS);
}

void deleteNodeForReplace(WebServer* webServer, UartPort* uartPort, Database* database) {
    // PARTE 2 de 3: BORRAR NODO ANTIGUO (guardando datos)
    uint16_t nodeNetAddress = getNodeNetAddress(replaceData.oldNodeID);
    sendLogCommissionEntry(webServer, "Looking for Node " + QString::number(nodeNetAddress) + " to delete...", "INFO");

    uint16_t nodeAddress = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
    replaceData.oldNodeRealAddress = nodeAddress;

    replaceNode = database->getNodeDataForReplace(nodeAddress);

    // Borrado del dispositivo elegido
    printf(" Net Address: %04X - RealAddress: %04X\n", nodeNetAddress, nodeAddress);

    sendUartInyectNode(uartPort, nodeAddress, database);
    while(messageState == PENDING) {}

    if(messageState == RECEIVED) {
        sendUartDelDevice(uartPort, nodeAddress, false);
        while(messageState == PENDING) {}
    }

    // Device to delete added to log
    insertDevToLog(nodeAddress, database, LOG_DEVICE_REMOVED, "Device");

    // Eliminar el nodo de la estructura interna
    meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].deleteDevice();
    database->deleteNode(nodeAddress);

    if(messageState == MISSED) {
        restoreDataForReplace(webServer, uartPort, database);
    }
}

void restoreDataForReplace(WebServer* webServer, UartPort* uartPort, Database* database) {
    // PARTE DE LA INSTALLKEY -----------------------
    sendLogCommissionEntry(webServer, "The node is configuring the installKey...", "INFO");

    database->clearUnassignedNodes();
    for(int n = 0; n < commissionedNodes.size(); n++) {
        database->deleteNode(meshDevice[commissionedNodes[n].i][commissionedNodes[n].j].getRealAddress());
        meshDevice[commissionedNodes[n].i][commissionedNodes[n].j].deleteDevice();

        database->addUnassignedNode(commissionedNodes[n].serial);
    }
    database->loadNodesFromDatabase();
    commissionedNodes.clear();

    database->doAutoAssignment();

    QList<UnassignedNode> unassignedNodes = database->getUnassignedNodes();

    // Unos 10s por nodo --> 6 nodos/min

    for(UnassignedNode unassignedNode : unassignedNodes)
    {
        uint8_t installKey[16] = {0};
        if(unassignedNode.installKey != "16")
        {
            memcpy(installKey, installKeys[unassignedNode.installKey.toInt() - 1], 16);
        }
        else
        {
            QString installKeyStr = database->getInstallKey();

            for (int i = 0; i < 16; ++i) {
                installKey[i] = static_cast<uint8_t>(installKeyStr.mid(i * 2, 2).toUInt(nullptr, 16));
            }
        }

        lastAssignedAddress = unassignedNode.bluetoothAddress;
        insertLogEvent(database, "Assign REQUEST [" + unassignedNode.installKey + "]", unassignedNode.serial, unassignedNode.bluetoothAddress, getAntennaInfo(database).ip, getAntennaInfo(database).timestamp, LOG_ASSIGNMENT_REQUEST, "Assignment");

        sendUartInstallKey(uartPort, database, unassignedNode.serial, unassignedNode.bluetoothAddress, installKey);
        delay(10000);
    }

    uint16_t newNextUnicastAddress = database->getMayorUnicastAddressOfUnassignedNodes();
    if(newNextUnicastAddress > 0)
        database->updateNextUnicastAddress(newNextUnicastAddress);
    database->clearUnassignedNodes();

    database->loadNodesFromDatabase();
    // PARTE DE LA INSTALLKEY -----------------------

    // PARTE 3 de 3: CARGAR DATOS AL NODO NUEVO
    sendLogCommissionEntry(webServer, "The node is loading the data...", "INFO");

    // Cargar los datos en el nodo (parte modelo)
    uint16_t nodeNetAddress = database->getNodeNetAddressForReplace(replaceData.newNodeRealAddress);

    Device &dev1 = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64];
    Device &dev2 = meshDevice[replaceNode.subnetAddress][replaceNode.nodeSubnetAddress];

    Device temp;
    temp.copyFrom(dev1); // dev1 -> temp
    dev1.copyFrom(dev2); // dev1 <- dev2
    dev2.copyFrom(temp); // dev2 <- temp

    uint16_t* removedGroups = dev2.delAllGroups();
    QStringList groups = replaceNode.groupSubAddress.split(", ");
    for(uint8_t i = 0; i < groups.size(); i++) {
        dev2.setGroupSubAddress(groups[i].toUShort(nullptr, 16));
    }

    // Cargar los datos en el nodo (parte BBDD)
    database->setNodeDataForReplace(replaceNode, replaceData.newNodeRealAddress);

    sendUartSetRelay(uartPort, replaceData.newNodeRealAddress, replaceNode.relayMode);
    delay(250);

    uint16_t* address = new uint16_t[2];
    address[0] = replaceData.newNodeRealAddress;
    for(uint8_t j = 0; j < MESH_GROUP_COUNT; j++) {
        if(removedGroups[j] != 0) {
            address[1] = removedGroups[j];
            sendUartDelGroupSimple(uartPort, address);
            delay(200);
        }
    }

    for(uint8_t k = 0; k < groups.size(); k++) {
        address[1] = groups[k].toUShort(nullptr, 16);

        timerGroupAddress[0] = address[0];
        timerGroupAddress[1] = address[1];
        timerGroupAddress[2] = 0x0000;

        groupDataConfiguration.configSecondGroup = false;
        sendUartAddGroupManual(uartPort, address);
        delay(2000);
    }

    sendLogCommissionEntry(webServer, "The node has loaded the data.", "INFO");
    delay(5000);
    sendUartConfirmReplacing(uartPort, replaceData.newNodeRealAddress);
    database->loadNodesFromDatabase();
}

void sendRecoveringMicro(WebServer* webServer)
{
    QString message = QString(WS_SEND_RECOVERING_MICRO) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
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

void sendLogCommissionEntry(WebServer* webServer, QString content, QString type)
{
    QString message = QString(WS_SEND_LOG_COMMISSION_ENTRY) + "@" + content + "_" + type;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmStartScan(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_START_SCAN) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmEndScan(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_END_SCAN) + "@" + " ";

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

void sendDevError(WebServer* webServer)
{
    QString message = QString(WS_SEND_DEVICE_ERROR) + "@" + " ";

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

    // Evitar que se añadar UUIDs duplicados
    for (uint8_t i = 0; i < 20; i++) {
        if (memcmp(scannedUUID[i].UUID, uuid, sizeof(uuid)) == 0) {
            return;
        }
    }

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
    QString message = QString(WS_SEND_SCANNED_DEVICES) + "@" + value + "_" + "true";

    qDebug() << "NODE SCANNED: " << value <<  " - REPORT ADDRESS: " << reportAddress;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendStoredScannedDevices(WebServer* webServer)
{
    uint8_t emptyUUID[16] = {0};
    QString value;

    for(uint8_t i = 0; i < 20; i++) {
        if (memcmp(scannedUUID[i].UUID, emptyUUID, sizeof(emptyUUID)) != 0) {
            value = "";

            for (uint8_t j = 0; j < 16 ; j++) { value += QString::asprintf("%02X", scannedUUID[i].UUID[j]); }

            QString message = QString(WS_SEND_SCANNED_DEVICES) + "@" + value + "_" + "false";

            if (webServer != nullptr) { webServer->sendData(message); }
        }
    }
}

void sendAddedDevices(QByteArray data, WebServer* webServer, Database* database)
{
    //uint8_t nodeUUID[16];
    //uint8_t netAddress[2];

    uint16_t nodeAddress = ((unsigned char)data[3] << 8) + (unsigned char)data[4];
    //for (uint8_t i = 0; i < 16; i++) { nodeUUID[i] = (unsigned char)data[5 + i]; }

    //setFirstAddressAvailable(nodeAddress, nodeUUID, database, netAddress);

    qDebug() << "UART FRAME RECEIVED: ADDED DEVICE " << nodeAddress;

    sendLogCommissionEntry(webServer, "The device has been added.", "INFO");

    //QString message = QString(WS_SEND_ADDED_DEVICES) + "@" + QString::number(netAddress[0] * 64 + netAddress[1] + 1);

    //if (webServer != nullptr) { webServer->sendData(message); }
}

void sendDeviceError(QByteArray data, UartPort* uartPort, WebServer* webServer, Database* database)
{
    uint8_t nodeUUID[16];

    commissionData.numberOfNodesAdded++;
    //numberOfIterations++;

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

    sendLogCommissionEntry(webServer, "An error has occurred with the device...", "ERROR");
    sendDevError(webServer); // mensaje de error de añadir device para add manual y commission

    database->updateNextUnicastAddress(database->getNextUnicastAddress() + 1);

    delay(5000);

    // PARA STOP_COMMISSION
    if(forceStopCommissioning)
    {
        commissionData.numberOfNodesScanned = commissionData.numberOfNodesAdded;
    }

    // PARA ADD_DEVICE MANUAL
    if(isManualAddingDevice)
    {
        // Recuperar la lista de nodos escaneados en scannedUUID
        memcpy(scannedUUID, scannedUUIDBackup, sizeof(scannedUUIDBackup));
        numberOfIterations = 0;
        doneIterations = 0;
        isManualAddingDevice = false;

        if(!isReplacingDevices) {
            sendUartClearInyectedNodes(uartPort, false, database);
            while(messageState == PENDING) {}

            sendConfirmAddingDevice(webServer); // mensaje de confirmación de añadir device (SOLO para el adding manual)
            cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
            embeddedState = FREE;
        } else {
            replaceP2Timer.start(300); // siguiente paso del replacing
        }

        return;
    }

    uint8_t emptyUUID[16] = {0};
    for (uint8_t l = 0; l < 20; l++) {
        if (commissionData.numberOfNodesScanned == commissionData.numberOfNodesAdded) {
            commissionData.numberOfNodesScanned = 0;
            commissionData.numberOfNodesAdded = 0;
            //uint16_t addressToNextIt = database->getNextNodeAddress(doneIterations);
            //qDebug() << "[2] DONE ITERATIONS: "<< doneIterations;
            //delay(300);
            //sendUartInyectNode(uartPort, addressToNextIt, database);
            //delay(500);
            //sendUartNewIteration(uartPort, addressToNextIt);
            //newIterationTimer.start(NEW_ITERATION_TIMER_MS);
            addDeviceTimer.start(100);
            break;
        }
        if (memcmp(scannedUUID[l].UUID, emptyUUID, sizeof(emptyUUID)) != 0) {
            qDebug() << "ADDING NEW NODE UUID" << QString("0x%1").arg(scannedUUID[l].UUID[0], 2, 16, QChar('0')).toUpper();
            sendUartAddDevice(uartPort, scannedUUID[l]);
            sendLogCommissionEntry(webServer, "Start adding node " + getUUIDAsString(scannedUUID[l].UUID), "INFO");
            confirmAddDeviceTimer.start(CONFIRM_ADD_DEVICE_TIMER_MS);
            break;
        }
    }
}

void sendNodesFromDatabase(WebServer* webServer, Database* database)
{
    QList<QString> nodeNetAddressAndSNList = database->getConfiguredNodesAndSerialNumbers();

    for (const QString& nodeInfo : nodeNetAddressAndSNList) {
        QStringList nodeInfoParts = nodeInfo.split("#");

        QString netAddress = nodeInfoParts[0];
        QString serialNumber = nodeInfoParts[1];
        bool relayStatus = nodeInfoParts[2].toInt() != 0;

        QString message = QString(WS_SEND_ADDED_DEVICES) + "@" + netAddress + "_" + serialNumber + "_" + (relayStatus ? "relayOn" : "relayOff") + "_" + + "false"; // el booleano indica que no se debe incrementar el contador del webserver
        if (webServer != nullptr) { webServer->sendData(message); }
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

void sendFactoryIDWrote(WebServer* webServer, bool received)
{
    QString message = QString(WS_SEND_FACTORY_ID_WROTE) + "@" + (received ? "true" : "false");

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendDaliTested(WebServer* webServer, bool received)
{
    QString message = QString(WS_SEND_DALI_TESTED) + "@" + (received ? "true" : "false");

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendRecordedDevice(WebServer* webServer, bool received)
{
    QString message = QString(WS_SEND_RECORDED_DEVICE) + "@" + (received ? "true" : "false");

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendSerialClosure(WebServer* webServer, bool done)
{
    QString message = QString(WS_SEND_SERIAL_CLOSURE) + "@" + (done ? "done" : "notDone");

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendIsConfig(WebServer* webServer, QString device, QString serialNumber, bool isConfig, bool hasFailures, bool onOffStatus, bool isInEmergency)
{
    QString message = QString(WS_SEND_IS_CONFIG) + "@" + device + "_" + serialNumber + "_" + (isConfig ? "true" : "false") + "_" + (hasFailures ? "true" : "false") + "_" + (onOffStatus ? "on" : "off") + "_" + (isInEmergency ? "on" : "off");

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendLogData(WebServer *webServer, QList<QStringList> logs)
{
    QString content;
    for(QStringList log : logs) {
        for(QString data : log) {
            content += (data + "|");
        }
        content += ("#");
    }

    QString message = QString(WS_SEND_LOG_DATA) + "@" + content;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendLogFile(WebServer *webServer, QString fileDir)
{
    QString message = QString(WS_SEND_LOG_FILE) + "@" + fileDir;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void clearSystemData(WebServer* webServer, Database* database, UartPort* uartPort)
{
    sendEstimatedTime(webServer, 30); // 30s de margen para que se borren todos los nodos

    uint16_t dummyAddress = 0xFFFF;
    sendUartClearAllData(uartPort, dummyAddress);
    while(messageState == PENDING) {}

    database->clearAllData();

    for (int i = 0; i < MAX_SUBNET; i++)
        for (int j = 0; j < MAX_NODES_SUBNET; j++)
            meshDevice[i][j].deleteDevice();

    for (int i = 0; i < MAX_TEST; i++)
        tests[i].deleteTest();
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

void sendConfirmStartRemoveOneNode(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_START_DEL_ONE_DEV) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmEndRemoveOneNode(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_END_DEL_ONE_DEV) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmAddNodeToGroup(WebServer* webServer, uint16_t address, uint16_t deviceTypeGroupAddress, bool added, Database* database)
{
    uint8_t subnetAddress, nodeSubnetAddress;

    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (meshDevice[i][j].getRealAddress() == address) {
                subnetAddress = i; nodeSubnetAddress = j;
                if(added) {
                    meshDevice[i][j].setGroupSubAddress(deviceTypeGroupAddress); // Añadir al modelo
                    database->setGroup(address, deviceTypeGroupAddress); // Añadir grupo en la BBDD
                    if (!pendingGroupUpdatesEth.isEmpty()) {
                        QString key = QString("%1:%2").arg(address).arg(deviceTypeGroupAddress);
                        pendingGroupUpdatesEth.remove(key);
                    }
                }
                break;
            }
        }
    }

    QString message = QString(WS_SEND_CONFIRM_ADD_NODE_TO_GROUP) + "@" + (added ? "true" : "false");

    if (webServer != nullptr && groupUpdateFromEth != true) { webServer->sendData(message); }

    // Log entry
    QString groupAddressString = QString("%1").arg(deviceTypeGroupAddress, 4, 16, QLatin1Char('0')).toUpper();
    int globalPos = subnetAddress * 64 + nodeSubnetAddress + 1;
    QString devname = "A" + QString::number(globalPos).rightJustified(4, '0');
    QString name = devname + " - " + database->getGroupName(groupAddressString);
    QString serialNum = meshDevice[subnetAddress][nodeSubnetAddress].serialNumberString();
    int btAddress = meshDevice[subnetAddress][nodeSubnetAddress].getRealAddress();;
    AntennaInfo info = getAntennaInfo(database);
    QString eventType = "Groups";
    insertLogEvent(database, name, serialNum, btAddress, info.ip, info.timestamp, added ? LOG_ADDED_TO_GROUP_OK : LOG_ADDED_TO_GROUP_FAIL, eventType);
}

void sendConfirmDelNodeFromGroup(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_DEL_NODE_FROM_GROUP) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmDelGroup(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_DEL_GROUP) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmSetRelay(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_MANUAL_RELAY) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmPowerOnLevel(WebServer* webServer, uint8_t powerOnLevel, uint16_t groupAddress, Database* database)
{
    QString groupAddressString = QString("%1").arg(groupAddress, 4, 16, QLatin1Char('0')).toUpper();

    database->setPowerOnLevel(groupAddressString, powerOnLevel);

    QString message = QString(WS_SEND_CONFIRM_POWER_ON_LEVEL) + "@" + groupAddressString + "_" + QString::number(powerOnLevel);

    if (webServer != nullptr) { webServer->sendData(message); }

    // Log entry
    QString name = database->getGroupName(groupAddressString) + " [G]";
    QString serialNum = "FF.FF.FF.FF";
    int btAddress =  groupAddress;
    AntennaInfo info = getAntennaInfo(database);
    int eventCode = powerOnLevel == 0 ? POL_OFF : (powerOnLevel == 254 ? POL_MAX : POL_LAST_VALUE);
    QString eventType = "PowerOnLevel";
    insertLogEvent(database, name, serialNum, btAddress, info.ip, info.timestamp, eventCode, eventType);
}

void sendFailComCycles(WebServer* webServer)
{
    QString message = QString(WS_SEND_FAIL_COM_CYCLES) + "@" + QString::number(failComCycles);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void updateRelayStatus(WebServer* webServer, Database* database, uint16_t address, bool enabled)
{
    qDebug() << "Node Address:" << address << "- RELAY:" << (enabled ? "Enabled" : "Disabled");

    database->updateRelayMode(address, enabled);

    uint16_t netAddress;
    bool found = false;

    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (meshDevice[i][j].getRealAddress() == address) {
                netAddress = i * 64 + j + 1;
                found = true;
                break;
            }
        }

        if(found) { break; } // Evitar recorrer innecesariamente tras encontrar
    }

    QString message = QString(WS_SEND_CONFIRM_SET_RELAY) + "@" + QString::number(netAddress) + "_" + (enabled ? "relayOn" : "relayOff");

    if (webServer != nullptr) { webServer->sendData(message); }
}

void reloadAntennaAddressAndInstallKey(WebServer* webServer, Database* database, uint16_t antennaAddress, const uint8_t* installKey)
{
    database->setMasterRealAddress(antennaAddress);
    antennaRealAddress = antennaAddress;

    QString installKeyStr = "";
    for(int i = 0; i < 15; i++)
        if(memcmp(installKeys[i], installKey, 16) == 0)
            installKeyStr = QString::number(i + 1);

    if(installKeyStr == "")
        for(int j = 0; j < 16; j++)
            installKeyStr += QString::asprintf("%02X", installKey[j]);

    QString currentInstallKey = database->getInstallKey();
    if(currentInstallKey != installKeyStr) {
        database->setInstallKey(installKeyStr);
        database->clearAllData();
    }
}

void updatePowerOnLevels(WebServer* webServer, Database* database, uint16_t nodeAddr, uint8_t powerOnLevel)
{
    qDebug() << "Node Address:" << QString::number(nodeAddr) << "- PowerOnLevel:" << QString::number(powerOnLevel);

    for(const QPair<uint16_t, QStringList> &par : crossedGroupAndNodes) {
        if (par.first == nodeAddr) {
            const QStringList &groupsList = par.second;

            for (const QString &group : groupsList) {
                database->setPowerOnLevel(group, powerOnLevel);
            }
            break;
        }
    }
}

void sendConfirmStartLineScanning(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_START_LS) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmEndLineScanning(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_END_LS) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmEndSyncPOL(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_END_SYNC_POL) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendLSInfo(WebServer* webServer, uint16_t nodeAddr, uint8_t phase)
{
    //QString hexStr = QString("0x%1").arg(nodeAddr, 4, 16, QChar('0')).toUpper();

    QString message = QString(WS_SEND_LS_INFO) + "@" + QString::number(nodeAddr) + "_" + QString::number(phase);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendFoundNodes(WebServer* webServer, uint16_t nodesCount)
{
    QString message = QString(WS_SEND_LS_FOUNDED) + "@" + QString::number(nodesCount);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void changePositions(Database* database, uint16_t pos1, uint16_t pos2)
{
    uint16_t indexIPos1 = (pos1 - 1) / 64;
    uint16_t indexJPos1 = (pos1 - 1) % 64;
    uint16_t indexIPos2 = (pos2 - 1) / 64;
    uint16_t indexJPos2 = (pos2 - 1) % 64;

    Device &dev1 = meshDevice[indexIPos1][indexJPos1];
    Device &dev2 = meshDevice[indexIPos2][indexJPos2];

    uint16_t realAddressDev1 = dev1.getRealAddress();
    uint16_t realAddressDev2 = dev2.getRealAddress();

    // Reemplazo en el modelo
    Device temp;
    temp.copyFrom(dev1); // dev1 -> temp
    dev1.copyFrom(dev2); // dev1 <- dev2
    dev2.copyFrom(temp); // dev2 <- temp

    // Reemplazo en la base de datos
    if(realAddressDev1 != 0x0000) { database->changePosition(indexIPos2, indexJPos2, realAddressDev1); }
    if(realAddressDev2 != 0x0000) { database->changePosition(indexIPos1, indexJPos1, realAddressDev2); }

    // update subnet count for eth
    if(!polling.isSubnetConfigured(indexIPos2)){
        polling.setConfiguredSubnets();
    }
}

void sendConfirmEndClearAllData(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_END_CLEAR_ALL) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmStartReplace(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_START_REPLACE) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmEndReplace(WebServer* webServer)
{
    QString message = QString(WS_SEND_CONFIRM_END_REPLACE) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendWriteIDError(WebServer* webServer)
{
    QString message = QString(WS_SEND_WRITE_ID_ERROR) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendEstimatedTime(WebServer* webServer, uint16_t time)
{
    uint8_t hours = time / 3600;
    uint16_t tmp = time % 3600;
    uint8_t minutes = tmp / 60;
    uint8_t seconds = tmp % 60;

    QString message = QString(WS_SEND_ESTIMATED_TIME) + "@" + QString::number(hours) + ":" + QString::number(minutes) + ":" + QString::number(seconds);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendInitAlert(WebServer* webServer)
{
    QString message = QString(WS_SEND_INIT_ALERT) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void processFactoryProgramSerial(UartPort* uartPort, QByteArray dataBuffer)
{
    isFactoryProgramOn = true;
    factoryProgramSerial = dataBuffer.split(';').value(1);

    cleanCdbTimer.stop();

    QString deviceID = factoryProgramSerial;
    qDebug() << "[ID_CODE] deviceID =" << deviceID;

    sendWriteIDCodeFrame(uartPort, deviceID);
    while(messageState == PENDING) {}

    if(messageState == RECEIVED) {
        delay(500);
        sendDaliTestForWriteID(uartPort, deviceID);
        while(messageState == PENDING) {}

        if(messageState == RECEIVED) {
            delay(500);
            sendEndRecordDevice(uartPort, deviceID);
            while(messageState == PENDING) {}
        }
    }

    // start del cleanCdbTimer en la respuesta al finalizar el escaneo
}

void sendUnassignedNodesPaged(WebServer* webServer, Database* database, uint16_t page)
{
    QString unassignedNodes = database->getUnassignedNodesPaged(page).join("#");

    QString message = QString(WS_SEND_UNASSIGNED_NODES) + "@" + QString::number(page) + "=" + unassignedNodes;

    if (webServer != nullptr) { webServer->sendData(message); }
}

void applyAutoAssignment(WebServer* webServer, UartPort* uartPort, Database* database)
{
    if(!database->allNodesHaveAutoAssignment()) { return; }

    embeddedState = APPLY_AUTOASSIGNMENT;
    cleanCdbTimer.stop();
    sendConfirmStartApplyAutoAssignment(webServer);

    // Se guarda la mayor direccion unicast de los nodos a asignar para que las acciones sucesivas no repitan dirección
    uint16_t newNextUnicastAddress = database->getMayorUnicastAddressOfUnassignedNodes();

    QList<UnassignedNode> unassignedNodes = database->getUnassignedNodes();

    int counter = 0;
    int totalNodes = unassignedNodes.size();
    sendNodeAutoAssignInfo(webServer, counter, totalNodes);

    // Unos 10s por nodo --> 6 nodos/min
    for(UnassignedNode unassignedNode : unassignedNodes)
    {
        uint8_t installKey[16] = {0};
        if(unassignedNode.installKey != "16")
        {
            memcpy(installKey, installKeys[unassignedNode.installKey.toInt() - 1], 16);
        }
        else
        {
            QString installKeyStr = database->getInstallKey();

            for (int i = 0; i < 16; ++i) {
                installKey[i] = static_cast<uint8_t>(installKeyStr.mid(i * 2, 2).toUInt(nullptr, 16));
            }
        }

        lastAssignedAddress = unassignedNode.bluetoothAddress;
        insertLogEvent(database, "Assign REQUEST [" + unassignedNode.installKey + "]", unassignedNode.serial, unassignedNode.bluetoothAddress, getAntennaInfo(database).ip, getAntennaInfo(database).timestamp, LOG_ASSIGNMENT_REQUEST, "Assignment");

        sendUartInstallKey(uartPort, database, unassignedNode.serial, unassignedNode.bluetoothAddress, installKey);
        sendNodeAutoAssignInfo(webServer, ++counter, totalNodes);
        delay(10000);
    }

    // Se vuelca la mayor dirección en el registro general de la BBDD
    if(newNextUnicastAddress > 0) { database->updateNextUnicastAddress(newNextUnicastAddress); }

    database->loadNodesFromDatabase();

    sendConfirmEndApplyAutoAssignment(webServer, database);
    cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
    embeddedState = FREE;
}

void sendConfirmStartApplyAutoAssignment(WebServer* webServer)
{
    QString message = QString(WS_SEND_START_APPLY_ASSIGN) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmEndApplyAutoAssignment(WebServer* webServer, Database* database)
{
    QString message = QString(WS_SEND_END_APPLY_ASSIGN) + "@" + QString::number(database->getUnassignedNodesCount());

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendConfirmStartGroupAutoAssignment(WebServer* webServer)
{
    QString message = QString(WS_SEND_START_GROUP_AUTO_ASSIGN) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void applyGroupAutoAssignment(WebServer* webServer, UartPort* uartPort, Database* database)
{
    int counter = 0;
    int totalNodes = 0;
    for(int i = 0; i < MAX_SUBNET; i++){
        for(int j = 0; j < MAX_NODES_SUBNET; j++) {
            Device& device = meshDevice[i][j];
            if(device.getIsConfigured())
                totalNodes++;
        }
    }
    sendGroupAutoAssignInfo(webServer, counter, totalNodes);

    for(int i = 0; i < MAX_SUBNET; i++){
        for(int j = 0; j < MAX_NODES_SUBNET; j++) {
            Device& device = meshDevice[i][j];
            if(device.getIsConfigured()) {
                // LOG INFO (General) ------------------------------------------------------------
                QString devname = "A" + QString::number(i * 64 + j + 1).rightJustified(4, '0');
                QString serialNum = meshDevice[i][j].serialNumberString();
                int btAddress = meshDevice[i][j].getRealAddress();
                AntennaInfo info = getAntennaInfo(database);
                QString eventType = "Groups";
                // -------------------------------------------------------------------------------

                counter++;
                sendUartInyectNode(uartPort, device.getRealAddress(), database);
                while(messageState == PENDING) {}

                if(messageState == RECEIVED) {
                    uint8_t devType = device.getDeviceType();
                    if(devType == 0x01 || devType == 0x00) // EMERGENCY or DEFAULT
                    {
                        if(!device.isOnGroupSubAddress(0xC001)) {
                            sendUartAddGroupAuto(uartPort, device.getRealAddress(), 0xC001);
                            while(messageState == PENDING) {}

                            if(messageState == RECEIVED) {
                                meshDevice[i][j].setGroupSubAddress(0xC001); // Añadir al modelo
                                database->setGroup(device.getRealAddress(), 0xC001); // Añadir grupo en la BBDD
                            }

                            QString groupAddressString = QString("%1").arg(0xC001, 4, 16, QLatin1Char('0')).toUpper();
                            QString name = devname + " - " + database->getGroupName(groupAddressString);
                            insertLogEvent(database, name, serialNum, btAddress, info.ip, info.timestamp, messageState == RECEIVED ? LOG_ADDED_TO_GROUP_OK : LOG_ADDED_TO_GROUP_FAIL, eventType);
                        }

                        if((j + 1) % 2 == 0) { // PAR
                            if(!device.isOnGroupSubAddress(0xC002)) {
                                sendUartAddGroupAuto(uartPort, device.getRealAddress(), 0xC002);
                                while(messageState == PENDING) {}

                                if(messageState == RECEIVED) {
                                    meshDevice[i][j].setGroupSubAddress(0xC002); // Añadir al modelo
                                    database->setGroup(device.getRealAddress(), 0xC002); // Añadir grupo en la BBDD
                                }

                                QString groupAddressString = QString("%1").arg(0xC002, 4, 16, QLatin1Char('0')).toUpper();
                                QString name = devname + " - " + database->getGroupName(groupAddressString);
                                insertLogEvent(database, name, serialNum, btAddress, info.ip, info.timestamp, messageState == RECEIVED ? LOG_ADDED_TO_GROUP_OK : LOG_ADDED_TO_GROUP_FAIL, eventType);
                            }
                        }
                        else { // IMPAR
                            if(!device.isOnGroupSubAddress(0xC003)) {
                                sendUartAddGroupAuto(uartPort, device.getRealAddress(), 0xC003);
                                while(messageState == PENDING) {}

                                if(messageState == RECEIVED) {
                                    meshDevice[i][j].setGroupSubAddress(0xC003); // Añadir al modelo
                                    database->setGroup(device.getRealAddress(), 0xC003); // Añadir grupo en la BBDD
                                }

                                QString groupAddressString = QString("%1").arg(0xC003, 4, 16, QLatin1Char('0')).toUpper();
                                QString name = devname + " - " + database->getGroupName(groupAddressString);
                                insertLogEvent(database, name, serialNum, btAddress, info.ip, info.timestamp, messageState == RECEIVED ? LOG_ADDED_TO_GROUP_OK : LOG_ADDED_TO_GROUP_FAIL, eventType);
                            }
                        }
                    }
                    else if (devType == 0x06) // LIGHTING
                    {
                        if(!device.isOnGroupSubAddress(0xC000)) {
                            sendUartAddGroupAuto(uartPort, device.getRealAddress(), 0xC000);
                            while(messageState == PENDING) {}

                            if(messageState == RECEIVED) {
                                meshDevice[i][j].setGroupSubAddress(0xC000); // Añadir al modelo
                                database->setGroup(device.getRealAddress(), 0xC000); // Añadir grupo en la BBDD
                            }

                            QString groupAddressString = QString("%1").arg(0xC000, 4, 16, QLatin1Char('0')).toUpper();
                            QString name = devname + " - " + database->getGroupName(groupAddressString);
                            insertLogEvent(database, name, serialNum, btAddress, info.ip, info.timestamp, messageState == RECEIVED ? LOG_ADDED_TO_GROUP_OK : LOG_ADDED_TO_GROUP_FAIL, eventType);
                        }
                    }
                    else
                    {
                        QString name = devname + " - " + "TYPE";
                        insertLogEvent(database, name, serialNum, btAddress, info.ip, info.timestamp, LOG_ADDED_TO_GROUP_FAIL, eventType);
                    }
                }
                else {
                    QString name = devname + " - " + "INJECTION";
                    insertLogEvent(database, name, serialNum, btAddress, info.ip, info.timestamp, LOG_ADDED_TO_GROUP_FAIL, eventType);
                }

                sendUartClearOneInyectedNode(uartPort, device.getRealAddress());
                while(messageState == PENDING) {}

                sendGroupAutoAssignInfo(webServer, counter, totalNodes);
            }
        }
    }

    sendUartClearInyectedNodes(uartPort, false, database);
    while(messageState == PENDING) {}

    delay(3000);
}

void sendConfirmEndGroupAutoAssignment(WebServer* webServer)
{
    QString message = QString(WS_SEND_END_GROUP_AUTO_ASSIGN) + "@" + " ";

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendGroupAutoAssignInfo(WebServer* webServer, int counter, int totalNodes)
{
    QString message = QString(WS_SEND_INFO_GROUP_AUTO_ASSIGN) + "@" + QString::number(counter) + "_" + QString::number(totalNodes);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendActiveKeyAndForcing(WebServer* webServer, uint8_t activeKey, bool forceInstallKey)
{
    QString message = QString(WS_SEND_ACTIVE_KEY_AND_FORCE) + "@" + QString::number(activeKey) + "_" + (forceInstallKey ? "forcing" : "notForcing");

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendAddUnassignedError(WebServer* webServer, uint8_t result)
{
    QString message = QString(WS_SEND_ADD_UNASSIGNED_ERROR) + "@" + QString::number(result);

    if (webServer != nullptr) { webServer->sendData(message); }
}

void sendNodeAutoAssignInfo(WebServer* webServer, int counter, int totalNodes)
{
    QString message = QString(WS_SEND_INFO_NODE_AUTO_ASSIGN) + "@" + QString::number(counter) + "_" + QString::number(totalNodes);

    if (webServer != nullptr) { webServer->sendData(message); }
}
