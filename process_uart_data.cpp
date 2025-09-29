#include "process_uart_data.h"
#include "process_webserver_data.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "time_functions.h"
#include "global_variables.h"
#include "log.h"

static bool checkCRC(QByteArray data)
{
    uint8_t checkSum, crc = 0;

    for (uint8_t i = 1; i < (data.length() - 1); i++) { crc += (unsigned char)data[i]; }

    checkSum = (unsigned char)data[data.length() - 1];

    if (crc == checkSum) { return true; }
    else { return false; }
}

int getExpectedFrameSize(const QByteArray& buffer)
{
    if (buffer.size() < 2) return -1;  // no hay FRAME_TYPE

    uint8_t frameType = static_cast<uint8_t>(buffer[1]);
    uint8_t subType = buffer.size() > 2 ? static_cast<uint8_t>(buffer[2]) : 0;

    switch (frameType) {
    case UART_RSP_CONFIG_FRAME_TYPE:
        switch (subType) {
            case SCAN_DEVICES: return 22;
            case CONFIRM_START_COMMISSION: return 4;
            case CONFIRM_ADD_DEVICE: return 4;
            case CONFIRM_GROUP_FRAME: return 4;
            case CONFIRM_NEW_ITERATION: return 4;
            case CONFIRM_CHANGE_RELAY: return 4;
            case ADD_DEVICES: return 6;
            case DEVICE_ERROR: return 20;
            case COMMISSION_FAIL: return 7;
            case SEND_RECOVERY_NODE: return 38;
            case FEATURES: return 47;
            case GROUP_ADDED: return 10;
            case DEBUG: return 5;
            case NODE_DELETED: return 6;
            case CONFIRM_START_SCAN: return 4;
            case CONFIRM_ADD_NODE_TO_GROUP: return 9;
            case CONFIRM_START_REMOVE_ALL_NODES: return 4;
            case CONFIRM_END_REMOVE_ALL_NODES: return 4;
            case CONFIRM_START_REMOVE_ONE_NODE: return 4;
            case CONFIRM_END_REMOVE_ONE_NODE: return 4;
            case CONFIRM_INYECT: return 4;
            case CONFIRM_ACTION: return 4;
            case CONFIRM_RESET_CDB: return 4;
            case RELAY_STATUS: return 7;
            case LINE_SCAN_SEND: return 22;
            case QUERY_RESPONSE: return 7;
            case CONFIRM_SET_POWER_ON_LEVEL: return 7;
            case SCAN_NODE_NOT_FOUND: return 6;
            case CONFIRM_GET_ANTENNA_ADDRESS: return 6;
            case CONFIRM_END_LINE_SCANNING: return 4;
            case CONFIRM_START_LINE_SCANNING: return 4;
            case SEND_FEATURES_STATUS: return 13;
            case CONFIRM_START_GROUPS_RECOVERY: return 4;
            case ANSWER_POWER_ON_LEVEL: return 7;
            case CONFIRM_END_GROUPS_RECOVERY: return 4;
            case RECOVERY_GROUPS: return 80;
            case CONFIRM_END_CLEAR_ALL_DATA: return 4;
            case CONFIRM_REPLACE_DONE: return 4;
            default: return -1;
        }

    case UART_RSP_CHANGE_FRAME_TYPE:
        return 7;

    case UART_RSP_POLLING_FRAME_TYPE:
        return 9;

    case UART_ID_FRAME_TYPE:
        switch (subType) {
            case FACTORY_ID_WROTE:
            case DALI_TESTED:
            case RECORDED_DEVICE:
                return 4;
            default: return -1;
        }

    default:
        return -1;
    }
}

void extractAndProcessFrames(const QByteArray& rawData, WebServer* webServer, UartPort* uartPort, Database* database)
{
    static QByteArray buffer;
    buffer.append(rawData);

    while (true) {
        // Buscar HEADER (0xAF)
        int headerIdx = buffer.indexOf((char)UART_HEADER);
        if (headerIdx == -1) {
            buffer.clear();  // No hay header → limpiar basura
            break;
        }

        // Eliminar bytes basura antes del header
        if (headerIdx > 0) {
            buffer.remove(0, headerIdx);
            headerIdx = 0;
        }

        // Asegurar que al menos hay HEADER + LEN
        if (buffer.size() < 2) {
            break;  // Esperar más datos
        }

        // Obtener longitud
        int totalFrameSize = getExpectedFrameSize(buffer);
        if (totalFrameSize == -1) {
            buffer.remove(0, 1);  // ignorar y seguir buscando
            continue;
        }

        if (buffer.size() < totalFrameSize) {
            break;  // Aún no llegó todo
        }

        // Trama completa detectada
        QByteArray frame = buffer.mid(0, totalFrameSize);

        if (checkCRC(frame)) {
            processUartData(frame, webServer, uartPort, database);
        } else {
            qWarning() << "Trama descartada por CRC inválido";
        }

        // Eliminar la trama procesada del buffer
        buffer.remove(0, totalFrameSize);
    }
}

void processUartData(QByteArray data, WebServer* webServer, UartPort* uartPort, Database* database)
{
    QString hexString;
    for (uint8_t i = 0; i < data.size(); i++) {
        hexString += QString::asprintf("%02X ", static_cast<unsigned char>(data[i]));
    }
    qDebug() << hexString.trimmed();
    //qDebug() << "DATA SIZE" << data.size();
    if (!data.isEmpty() && (unsigned char)data[0] == UART_HEADER) {
        switch ((unsigned char)data[1]) {
            case UART_RSP_CONFIG_FRAME_TYPE:
                switch ((unsigned char)data[2]) {
                    case SCAN_DEVICES:
                        sendScannedDevices(data, webServer);
                    break;

                    case CONFIRM_START_COMMISSION:
                        //qDebug() << "PRUEBA UART";
                        qDebug() << "PARANDO TIMER START COMMISSION";
                        addDeviceTimer.start(ADD_DEVICE_TIMER_MS);
                        isCommissioning = true;  // Lock server from accepting new commands
                        sendConfirmStartCommission(webServer); 
                    break;

                    case CONFIRM_START_SCAN:
                        qDebug() << "CONFIRM START SCAN";
                        isScanning = true;
                        sendConfirmStartScan(webServer);
                        messageState = RECEIVED;
                        QTimer::singleShot(12000, []() {isScanning = false;});
                    break;

                    case CONFIRM_ADD_DEVICE:
                        qDebug() << "PARANDO TIMER ADD DEVICE";
                        sendLogCommissionEntry(webServer, "Add device command received...", "INFO");
                        confirmAddDeviceTimer.stop();
                        //sendConfirmAddingDevice(webServer);
                    break;

                    case CONFIRM_GROUP_FRAME:
                        qDebug() << "PARANDO TIMER GROUP FRAME";
                        sendLogCommissionEntry(webServer, "Adding node to groups...", "INFO");
                        groupFrameTimer.stop();
                    break;

                    case CONFIRM_NEW_ITERATION:
                        //commissionData.numberOfNodesScanned--;
                        qDebug() << "PARANDO TIMER NEW ITERATION";
                        newIterationTimer.stop();
                        doneIterations++;
                        addDeviceTimer.start(ADD_DEVICE_TIMER_MS);
                    break;

                    case CONFIRM_CHANGE_RELAY:
                        qDebug() << "CONFIRM CHANGE RELAY";
                        sendLogCommissionEntry(webServer, "Confirm change relay...", "INFO");
                        commissionData.isChangeRelayConfirmed = true;
                    break;

                    case ADD_DEVICES:
                        currentNodeAddress = ((unsigned char)data[3] << 8) + (unsigned char)data[4];
                        sendAddedDevices(data, webServer, database);
                    break;

                    case DEVICE_ERROR:
                    {
                        qDebug() << "DEVICE ERROR";
                        QByteArray uuidBytes = data.mid(3,16);
                        insertCommissionErrorToLog(uuidBytes, database, LOG_COMMISSION_DEVICE_ERROR);
                        sendDeviceError(data, uartPort, webServer, database);
                    }
                    break;

                    case COMMISSION_FAIL:
                    {
                        uint16_t nodeAddress = ((unsigned char)data[3] << 8) | (unsigned char)data[4];
                        uint8_t failType = (uint8_t)data[5];
                        if(failType == GROUP_FAIL) {
                            sendLogCommissionEntry(webServer, "Error assigning node to group...", "ERROR");
                            insertDevToLog(nodeAddress, database, LOG_COMMISSION_GROUP_FAIL, "Commissioning");
                        }
                        else if(failType == DEV_TYPE_FAIL) {
                            sendLogCommissionEntry(webServer, "Error reading device type...", "ERROR");
                            insertDevToLog(nodeAddress, database, LOG_COMMISSION_DEV_TYPE_FAIL, "Commissioning");
                        }
                        else if(failType == NET_ADDR_FAIL) {
                            // No va a entrar por aquí porque el error de netAddress se procesa automáticamente en processGroupAddedFrame
                            // Se detecta cuando esa función recibe dos 0 como direcciones de grupo. Se procesa ahí para permitir que acabe el commission
                            //sendLogCommissionEntry(webServer, "Error assigning net address...", "ERROR");
                            //insertDevToLog(nodeAddress, database, LOG_COMMISSION_NET_ADDR_FAIL, "Commissioning");
                        }
                    }
                    break;

                    case FEATURES:
                        processFeaturesFrame(data, uartPort, database, webServer);
                    break;
                    case GROUP_ADDED:
                        processGroupAddedFrame(data, uartPort, database, webServer);
                    break;

                    case DEBUG:
                        qDebug() << "DEBUG FRAME:" << QString("0x%1").arg((unsigned char)data[3], 2, 16, QChar('0')).toUpper();
                    break;

                    case NODE_DELETED:
                    {
                        uint16_t nodeAddress = ((unsigned char)data[3] << 8) | (unsigned char)data[4];

                        qDebug() << "Nodo eliminado confirmado desde micro: " << nodeAddress;

                        // Borrar de meshDevice local
                        for (int i = 0; i < MAX_SUBNET; i++) {
                            for (int j = 0; j < MAX_NODES_SUBNET; j++) {
                                if(meshDevice[i][j].getRealAddress() == nodeAddress) {
                                    meshDevice[i][j].deleteDevice();
                                    database->deleteNode(nodeAddress);
                                    break;
                                }
                            }
                        }
                    }
                    break;

                    case CONFIRM_START_REMOVE_ALL_NODES:
                        sendConfirmStartRemoveAllNodes(webServer);
                    break;

                    case CONFIRM_END_REMOVE_ALL_NODES:
                        sendConfirmEndRemoveAllNodes(webServer);
                    break;

                    case CONFIRM_START_REMOVE_ONE_NODE:
                    {
                        if(!isReplacingDevices) {
                            //sendConfirmStartRemoveOneNode(webServer);
                        } else {
                            sendLogCommissionEntry(webServer, "Deleting node from the network...", "INFO");
                        }
                    }
                    break;

                    case CONFIRM_END_REMOVE_ONE_NODE:
                    {
                        messageState = RECEIVED;
                        if(!isReplacingDevices) {
                            //sendConfirmEndRemoveOneNode(webServer);
                        } else {
                            replaceP3Timer.start(3000); // siguiente paso del replacing
                        }
                    }
                    break;
                    case CONFIRM_INYECT:
                    {
                        messageState = RECEIVED;
                    }
                    break;
                    case CONFIRM_ACTION:
                    {
                        messageState = RECEIVED;
                    }
                    break;
                    case CONFIRM_RESET_CDB:
                    {
                        messageState = RECEIVED;
                    }
                    break;
                    case CONFIRM_GET_ANTENNA_ADDRESS:
                    {
                        uint16_t antennaAddress = ((uint16_t)data[3] << 8) | data[4];
                        reloadAntennaAddress(webServer, database, antennaAddress);
                        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
                    }
                    break;

                    case CONFIRM_REPLACE_DONE:
                    {
                        replaceData = {"", 0x0000, "", 0x0000};
                        replaceNode = {0x00, 0x00, "", 0x00, 0x0000};

                        isReplacingDevices = false;
                        sendUartClearInyectedNodes(uartPort, false, database);
                        while(messageState == PENDING) {}

                        sendConfirmEndReplace(webServer);
                        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
                        embeddedState = FREE;
                    }
                    break;

                    case RELAY_STATUS:
                    {
                        uint16_t nodeAddress = ((uint16_t)data[3] << 8) | data[4];
                        bool enabled = ((uint8_t)data[5] != 0);
                        updateRelayStatus(webServer, database, nodeAddress, enabled);
                    }
                    break;

                    case CONFIRM_START_LINE_SCANNING:
                    {
                        isLineScanning = true;
                        configuredNodes = database->getConfiguredNodes();
                        lineScanningCounter = 0;
                        scannedNodesCounter = 0;
                        discovered_nodes_count = 0;

                        forceStopLS1 = false;
                        forceStopLS2 = false;

                        for (int i = 0; i < MAX_SUBNET; i++) {
                            for (int j = 0; j < MAX_NODES_SUBNET; j++) {
                                meshDevice[i][j].deleteDevice();
                            }
                        }

                        messageState = RECEIVED;
                    }
                    break;
                    case SEND_RECOVERY_NODE:
                    {
                        //isLineScanning = true;
                        uint16_t nodeAddress = ((uint16_t)data[3] << 8) | data[4];

                        uint8_t uuid[16];
                        memcpy(uuid,
                               reinterpret_cast<const uint8_t*>(data.constData()) + 5,
                               sizeof(uuid));

                        uint8_t dev_key[16];
                        memcpy(dev_key,
                               reinterpret_cast<const uint8_t*>(data.constData()) + 21,
                               sizeof(dev_key));


                        while(configuredNodes.contains(lineScanningCounter + 1)) {
                            lineScanningCounter++;
                        }

                        database->setRecoveryNode(lineScanningCounter / 64, lineScanningCounter % 64, nodeAddress, uuid);
                        database->updateNextUnicastAddress(nodeAddress);

                        lineScanningCounter++;
                        scannedNodesCounter++;

                        discovered_nodes[discovered_nodes_count++] = nodeAddress;

                        sendFoundNodes(webServer, scannedNodesCounter);

                        database->setRecoveryDevKey(nodeAddress, dev_key);
                    }
                    break;
                    case SEND_FEATURES_STATUS:
                        qDebug() << "SEND_FEATURES_STATUS";
                        //isLineScanning = true;
                        processRecoveryFeaturesFrame(data, database);
                    break;

                    case RECOVERY_GROUPS:
                    {
                        uint16_t nodeAddress = ((uint16_t)data[3] << 8) | (uint16_t)data[4];
                        uint8_t validCount = (uint8_t)data[5];

                        for (uint8_t i = 0; i < validCount; i++) {
                            uint16_t groupAddr = (uint16_t)data[6 + 2*i]
                                                 | ((uint16_t)data[7 + 2*i] << 8);

                            database->setGroup(nodeAddress, groupAddr);

                            qDebug() << "RECOVERY_GROUPS: nodo=0x"
                                     << QString::asprintf("%04X", nodeAddress)
                                     << "→ añadiendo grupo 0x"
                                     << QString::asprintf("%04X", groupAddr);
                        }
                        break;
                    }

                    case CONFIRM_END_LINE_SCANNING:
                    {
                        database->loadNodesFromDatabase();

                        isLineScanning = false;
                        messageState = RECEIVED;
                    }
                    break;

                    case SCAN_NODE_NOT_FOUND:
                    {
                        uint16_t nodeAddr = (data[3] << 8) | data[4];
                        qDebug() << "Nodo no encontrado en base de datos:" << QString::asprintf("0x%04X", nodeAddr);
                        messageState = RECEIVED;
                    }
                    break;
                    case ANSWER_POWER_ON_LEVEL:
                    {
                        uint16_t nodeAddr = (data[3] << 8) | data[4];
                        uint8_t powerOnLevel = (uint8_t)data[5];
                        updatePowerOnLevels(webServer, database, nodeAddr, powerOnLevel);
                        messageState = RECEIVED;
                    }
                    break;
                    case CONFIRM_ADD_NODE_TO_GROUP:
                    {
                        uint16_t address = ((uint16_t)data[3] << 8) | data[4];
                        uint16_t deviceTypeGroupAddress = ((uint16_t)data[5] << 8) | data[6];
                        bool added = ((uint8_t)data[7] != 0); // si se añade o falla porque el micro es incapaz

                        if(added) { messageState = RECEIVED; }
                        else { messageState = MISSED; } // se simula que no ha llegado para que notifique el error en el proceso
                    }
                    break;

                    case CONFIRM_SET_POWER_ON_LEVEL:
                    {
                        uint16_t groupAddress = ((uint16_t)data[3] << 8) | data[4];
                        uint8_t powerOnLevel = data[5];
                        sendConfirmPowerOnLevel(webServer, powerOnLevel, groupAddress, database);
                    }
                    break;

                    case CONFIRM_END_CLEAR_ALL_DATA:
                    {
                        sendConfirmEndClearAllData(webServer); // no se usa ya que no devuelve confirmación al terminar
                    }
                    break;
                }
            case UART_RSP_CHANGE_FRAME_TYPE:
                processChangeFrame(data, database, webServer);
            break;

            case UART_RSP_POLLING_FRAME_TYPE:
                processPollingFrame(data);
            break;

            case UART_ID_FRAME_TYPE:
                switch ((unsigned char)data[2]) {
                    case FACTORY_ID_WROTE:
                        sendFactoryIDWrote(webServer);
                    break;

                    case DALI_TESTED:
                        sendDaliTested(webServer);
                    break;

                    case RECORDED_DEVICE:
                        sendRecordedDevice(webServer);
                    break;

                    default:
                    break;
                }
            break;

            default:
                break;
        }
    }
}

void processFeaturesFrame(QByteArray data, UartPort* uartPort, Database* database, WebServer* webServer)
{
    sendLogCommissionEntry(webServer, "Start loading features...", "INFO");

    uint8_t deviceType, ratedDuration, emergencyFeatures, physicalMinLvl;
    uint8_t nodeUUID[16];
    uint16_t address;
    uint16_t fatherAddress;
    uint16_t net_idx;
    uint8_t num_elem;
    uint8_t dev_key[16];

    address = ((unsigned char)data[3] << 8) + (unsigned char)data[4];
    QString value = "";

    for (uint8_t i = 0; i < 16; i++) {
        nodeUUID[i] = (unsigned char)data[5 + i];
        value += QString::asprintf("%02X", nodeUUID[i]);
    }

    deviceType = (unsigned char)data[21] == 0 ? 1 : (unsigned char)data[21]; // Por defecto, tipo 1 (emergencia)
    ratedDuration = (unsigned char)data[22];
    emergencyFeatures = (unsigned char)data[23];
    physicalMinLvl = (unsigned char)data[24];
    fatherAddress = ((unsigned char)data[25] << 8) + (unsigned char)data[26];

    net_idx = ((unsigned char)data[27] << 8) + (unsigned char)data[28];
    num_elem = (unsigned char)data[29];
    QString valueDK = "";
    for (uint8_t i = 0; i < 16; i++) {
        dev_key[i] = (unsigned char)data[30 + i];
        valueDK += QString::asprintf("%02X", dev_key[i]);
    }

    qDebug() << "FEATURES" << value << "FRAME:" << address << deviceType << ratedDuration << emergencyFeatures << physicalMinLvl << fatherAddress;
    //qDebug() << "Extra:" << dev_key << net_idx << num_elem;

    commissionData.numberOfNodesAdded++;
    numberOfIterations++;

    uint8_t revertedNodeUUID[16];
    // Revertir el UUID para que el serialNumber quede al final al añadir el nodo al modelo
    for (int8_t i = 15; i >= 0; i--) { revertedNodeUUID[i] = nodeUUID[15-i]; }

    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (!meshDevice[i][j].getIsConfigured() || memcmp(meshDevice[i][j].getUUID(), revertedNodeUUID, sizeof(meshDevice[i][j].getUUID())) == 0){
                meshDevice[i][j].setRealAddress(address);
                meshDevice[i][j].setUUID(revertedNodeUUID);
                meshDevice[i][j].setDeviceType(deviceType);
                meshDevice[i][j].setRatedDuration(ratedDuration);
                meshDevice[i][j].setEmergencyFeatures(emergencyFeatures);
                meshDevice[i][j].setPhysicalMinLvl(physicalMinLvl);
                meshDevice[i][j].setIsConfigured(true);

                netAddress = i * 64 + j + 1;

                database->setNewNode(i, j, address, nodeUUID, fatherAddress);
                database->updateNextUnicastAddress(address);
                insertDevToLog(meshDevice[i][j].getRealAddress(), database, LOG_DEVICE_ADDED, "Device");
                if(isReplacingDevices) { replaceData.newNodeRealAddress = address; }
                delay(500);
                
                /*
                QString valu = "";
                for (uint8_t l = 0; l < 20; l++)  {
                    for (int8_t p = 15; p >= 0; p--) { valu += QString::asprintf("%02X", scannedUUID[l].UUID[p]); }
                    qDebug() << "UUID" << valu;
                    valu = "";
                    delay(200);
                }
                */
                for (uint8_t k = 0; k < 20; k++) {
                    if (memcmp(scannedUUID[k].UUID, nodeUUID, sizeof(scannedUUID[k].UUID)) == 0) {
                        memset(scannedUUID[k].UUID, 0, sizeof(scannedUUID[k].UUID));
                        scannedUUID[k].nodeAddressReport = 0;
                        break;
                    }
                }

                database->setNodeFeatures(address, deviceType, ratedDuration, emergencyFeatures, physicalMinLvl, false);
                database->setExtraFeatures(address, net_idx, num_elem, dev_key);

                /*
                QString message = QString(WS_SEND_ADDED_DEVICES) + "@" + QString::number(i * 64 + j + 1);
                if (webServer != nullptr) { webServer->sendData(message); }
                //*/

                qDebug()  << "NODO AÑADIDO";
                sendLogCommissionEntry(webServer, "The features have been loaded.", "INFO");

                delay(4000);
                /*
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
                        sendLogCommissionEntry(webServer, "Start adding node " + getUUIDAsString(scannedUUID[l].UUID, "INFO"));
                        confirmAddDeviceTimer.start(CONFIRM_ADD_DEVICE_TIMER_MS);
                        break;
                    }
                }
                return;
                //*/
                //*
                timerGroupAddress[0] = meshDevice[i][j].getRealAddress();
                if (deviceType == 0x01) {               // EMERGENCY
                    timerGroupAddress[1] = 0xC001;

                    if(((j + 1) % 2) != 0) {
                        timerGroupAddress[2] = 0xC003;
                        qDebug() << "GROUP IMPAR";
                    }
                    else {
                        timerGroupAddress[2] = 0xC002;
                        qDebug() << "GROUP PAR";
                    }
                }
                else if (deviceType == 0x06) {          // LIGHTING
                    timerGroupAddress[1] = 0xC000;

                    if(((j + 1) % 2) != 0) {
                        timerGroupAddress[2] = 0xC003;
                        qDebug() << "GROUP IMPAR";
                    }
                    else {
                        timerGroupAddress[2] = 0xC002;
                        qDebug() << "GROUP PAR";
                    }
                }
                else {                                  // DEFAULT or UNKNOWN DEV TYPE -> Se mete en emergency
                    if(((j + 1) % 2) != 0) {
                        timerGroupAddress[2] = 0xC003;
                        qDebug() << "GROUP IMPAR";
                    }
                    else {
                        timerGroupAddress[2] = 0xC002;
                        qDebug() << "GROUP PAR";
                    }

                    timerGroupAddress[1] = 0xC001;
                }
                groupFrameTimer.start(GROUP_FRAME_TIMER_MS);
                sendUartAddGroup(uartPort, timerGroupAddress);
                return;
                //*/
            }
        }
    }
}

void processRecoveryFeaturesFrame(QByteArray data, Database* database)
{
    uint8_t deviceType, ratedDuration, emergencyFeatures, physicalMinLvl, relayMode;
    uint16_t address;
    uint16_t fatherAddress;
    address = ((unsigned char)data[3] << 8) + (unsigned char)data[4];

    deviceType = (unsigned char)data[5] == 0 ? 1 : (unsigned char)data[5]; // Por defecto, tipo 1 (emergencia)
    ratedDuration = (unsigned char)data[6];
    emergencyFeatures = (unsigned char)data[7];
    physicalMinLvl = (unsigned char)data[8];
    relayMode = (unsigned char)data[9];
    fatherAddress = ((unsigned char)data[10] << 8) + (unsigned char)data[11];

    qDebug() << "EXT FEATURES FRAME:" << address << deviceType << ratedDuration << emergencyFeatures << physicalMinLvl << fatherAddress;

    database->setNodeFeatures(address, deviceType, ratedDuration, emergencyFeatures, physicalMinLvl, relayMode);
    database->setFatherRealAddress(address, fatherAddress);

    qDebug()  << "NODO RECOVERY AÑADIDO A BASE DE DATOS";

    delay(2000);

    return;
}

void processGroupAddedFrame(QByteArray data, UartPort* uartPort, Database* database, WebServer* webServer)
{
    uint16_t nodeAddress, deviceTypeGroupAddress, netAdressGroupAddress;

    nodeAddress = ((unsigned char)data[3] << 8) + (unsigned char)data[4];
    deviceTypeGroupAddress = ((unsigned char)data[5] << 8) + (unsigned char)data[6];
    netAdressGroupAddress = ((unsigned char)data[7] << 8) + (unsigned char)data[8];

    qDebug() << "GROUP ADDED FRAME: " << nodeAddress << deviceTypeGroupAddress << netAdressGroupAddress;
    if (deviceTypeGroupAddress != 0x0000) { database->setGroup(nodeAddress, deviceTypeGroupAddress); }
    if (netAdressGroupAddress != 0x0000) { database->setGroup(nodeAddress, netAdressGroupAddress); }

    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (meshDevice[i][j].getRealAddress() == nodeAddress) {
                if (deviceTypeGroupAddress != 0x0000) { meshDevice[i][j].setGroupSubAddress(deviceTypeGroupAddress); }
                if (netAdressGroupAddress != 0x0000) { meshDevice[i][j].setGroupSubAddress(netAdressGroupAddress); }
                if(isCommissioning) {
                    QString message = QString(WS_SEND_ADDED_DEVICES) + "@" + QString::number(netAddress) + "_" + meshDevice[i][j].serialNumberString() + "_" + "relayOff" + "_" + "true"; // el booleano indica que se debe incrementar el contador del webserver
                    if (webServer != nullptr) { webServer->sendData(message); }
                }
                break;
            }
        }
    }

    netAddress = 0;

    qDebug()  << "NODO AÑADIDO";
    if(deviceTypeGroupAddress != 0x0000 || netAdressGroupAddress != 0x0000) {
        sendLogCommissionEntry(webServer, "The groups have been setted.", "INFO");
    }
    delay(4000);

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
    for (uint8_t i = 0; i < 20; i++) {
        if (commissionData.numberOfNodesScanned == commissionData.numberOfNodesAdded) {
            commissionData.numberOfNodesScanned = 0;
            commissionData.numberOfNodesAdded = 0;
            //uint16_t addressToNextIt = database->getNextNodeAddress(doneIterations);
            //qDebug() << "[3] DONE ITERATIONS: "<< doneIterations;
            //delay(300);
            //sendUartInyectNode(uartPort, addressToNextIt, database);
            //delay(500);
            //sendUartNewIteration(uartPort, addressToNextIt);
            //newIterationTimer.start(NEW_ITERATION_TIMER_MS);
            addDeviceTimer.start(100);
            break;
        }
        if (memcmp(scannedUUID[i].UUID, emptyUUID, sizeof(emptyUUID)) != 0) {
            qDebug() << "ADDING NEW NODE UUID" << QString("0x%1").arg(scannedUUID[i].UUID[0], 2, 16, QChar('0')).toUpper();
            sendUartAddDevice(uartPort, scannedUUID[i]);
            sendLogCommissionEntry(webServer, "Start adding node " + getUUIDAsString(scannedUUID[i].UUID), "INFO");
            confirmAddDeviceTimer.start(CONFIRM_ADD_DEVICE_TIMER_MS);
            break;
        }
    }
    return;

}

void processChangeFrame(QByteArray data, Database* database, WebServer* webServer)
{
    uint8_t daliCommandType, daliRegisterValue;
    uint16_t address;

    address = ((unsigned char)data[2] << 8) + (unsigned char)data[3];
    daliCommandType = (unsigned char)data[4];
    daliRegisterValue = (unsigned char)data[5];

    qDebug() << "CHANGE FRAME FROM " << address << ": " << daliCommandType << daliRegisterValue;

    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (meshDevice[i][j].getRealAddress() == address) {
                if (daliCommandType == QUERY_EMERGENCY_FEATURES) {
                    meshDevice[i][j].setEmergencyFeatures(daliRegisterValue);
                    database->setNodeRegister("EmergencyFeatures", meshDevice[i][j].getRealAddress(), daliRegisterValue);
                }
                else if (daliCommandType == QUERY_PHYSICAL_MIN_LVL) {
                    meshDevice[i][j].setPhysicalMinLvl(daliRegisterValue);
                    database->setNodeRegister("PhysicalMinLvl", meshDevice[i][j].getRealAddress(), daliRegisterValue);
                }
                else if (daliCommandType == QUERY_ACTUAL_LVL) { meshDevice[i][j].setActualLvl(daliRegisterValue); }
                else if (daliCommandType == QUERY_STATUS) { meshDevice[i][j].setControlGearStatus(daliRegisterValue); }
                else if (daliCommandType == QUERY_EMERGENCY_MODE) { meshDevice[i][j].setEmergencyMode(daliRegisterValue); }
                else if (daliCommandType == QUERY_FAILURE_STATUS) { meshDevice[i][j].setEmergencyFailureStatus(daliRegisterValue); }

                if (lastNetAddressClicked != 0 && isOpenNodeControl) {
                    if (address == meshDevice[(lastNetAddressClicked - 1) / 64][(lastNetAddressClicked - 1) % 64].getRealAddress()) {
                        sendNodeInfo(webServer, QString::number(lastNetAddressClicked));
                    }
                }

                return;
            }
        }
    }
}

void processPollingFrame(QByteArray data)
{
    uint8_t actualLvl, controlGearStatus, emergencyMode, emergencyFailureStatus;
    uint16_t address;

    address = ((unsigned char)data[2] << 8) + (unsigned char)data[3];
    actualLvl = (unsigned char)data[4];
    controlGearStatus = (unsigned char)data[5];
    emergencyMode = (unsigned char)data[6];
    emergencyFailureStatus = (unsigned char)data[7];

    qDebug() << "POLLING FRAME FROM " << address << ": " << actualLvl << controlGearStatus << emergencyMode << emergencyFailureStatus;

    pollingData.pollingReceived = true;

    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (meshDevice[i][j].getRealAddress() == address) {
                meshDevice[i][j].setActualLvl(actualLvl);
                meshDevice[i][j].setControlGearStatus(controlGearStatus);
                meshDevice[i][j].setEmergencyMode(emergencyMode);
                meshDevice[i][j].setEmergencyFailureStatus(emergencyFailureStatus);
                return;
            }
        }
    }
}


void processConfirmGroupFrame(void)
{
    qDebug() << "CONFIRM GROUP FRAME";
    groupFrameTimer.stop();
}

void sendUartInyectNode(UartPort* _uartPort, uint16_t nodeAddress, Database* database)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {500, 2000, 4000};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        uint8_t devKey[16] = {0};
        QString devKeyStr = database->getDevKey(nodeAddress);
        convertDevKeyStringToByteArray(devKeyStr, devKey);

        QByteArray frame;
        unsigned char length = 21;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(INYECT_NODE);
        frame.append((nodeAddress >> 8) & 0xFF);
        frame.append(nodeAddress & 0xFF);
        for (uint8_t i = 0; i < 16; i++) {
            frame.append(devKey[i]);
        }
        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del INYECT_NODE";
    }
}

void sendUartClearInyectedNodes(UartPort* _uartPort, bool isCommissioning, Database* database)
{
    uint16_t masterStoredAddress = database->getMasterRealAddress();
    QString netKey = database->getNetKey();
    uint8_t netKeyBytes[16];

    if(netKey.size() == 32) {
        for(uint8_t i = 0; i < 16; i++) {
            QString byteString = netKey.mid(i * 2, 2);
            netKeyBytes[i] = static_cast<uint8_t>(byteString.toUInt(nullptr, 16));
        }
    } else {
        memcpy(netKeyBytes, netKeys[netKey.toInt() - 1], 16);
    }

    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {2000, 3000, 4000};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;
        unsigned char length = 22;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(CLEAR_INYECTED_NODES);
        frame.append(isCommissioning ? 0x01 : 0x00);
        frame.append((masterStoredAddress >> 8) & 0xFF);
        frame.append(masterStoredAddress & 0xFF);
        for(uint8_t j = 0; j < 16; j++) {
            frame.append(netKeyBytes[j]);
        }
        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del CLEAR_INYECTED_NODES";
    }
}

void sendUartClearOneInyectedNode(UartPort* _uartPort, uint16_t nodeAddress)
{
    QByteArray frame;
    unsigned char length = 5;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(CLEAR_ONE_INYECTED_NODE);
    frame.append((nodeAddress >> 8) & 0xFF);
    frame.append(nodeAddress & 0xFF);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartUpdateNextUnicast(UartPort* _uartPort, uint16_t nextUnicastAddress)
{
    QByteArray frame;
    unsigned char length = 5;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(UPDATE_NEXT_UNICAST);
    frame.append((nextUnicastAddress >> 8) & 0xFF);
    frame.append(nextUnicastAddress & 0xFF);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartScannedDevices(UartPort* _uartPort)
{
    QByteArray frame;
    unsigned char length = 3;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(SCAN_DEVICES);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartStartCommission(UartPort* _uartPort, uint16_t nextUnicastAddress)
{
    QByteArray frame;
    unsigned char length = 5;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(START_COMMISSION);
    frame.append((nextUnicastAddress >> 8) & 0xFF);
    frame.append(nextUnicastAddress & 0xFF);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartNewIteration(UartPort* _uartPort, uint16_t addressToNextIt)
{
    QByteArray frame;
    unsigned char length = 5;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(NEW_ITERATION);
    frame.append((addressToNextIt >> 8) & 0xFF);
    frame.append(addressToNextIt & 0xFF);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartChangeRelay(UartPort* _uartPort, uint16_t nodeAddress, Database* database)
{
    sendUartInyectNode(_uartPort, nodeAddress, database);
    delay(300);

    QByteArray frame;
    unsigned char length = 5;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(CHANGE_RELAY);
    frame.append((nodeAddress >> 8) & 0xFF);
    frame.append(nodeAddress & 0xFF);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartAddDevice(UartPort* _uartPort, ScannedUUID uuidScanned)
{
    if(forceStopCommissioning) { return; }

    QByteArray frame;
    unsigned char length = 21;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(ADD_DEVICES);
    frame.append((uuidScanned.nodeAddressReport >> 8) & 0xFF);
    frame.append(uuidScanned.nodeAddressReport & 0xFF);

    QString value = "";
    for (uint8_t i = 0; i < 16; i++) {
        frame.append(uuidScanned.UUID[i]);
        value += QString::asprintf("%02X", uuidScanned.UUID[i]);
    }

    frame.append(UART_END);

    qDebug() << "ENVIO ADD DEVICE UART" << value;

    _uartPort->sendData(frame);
}

void sendUartDelDevice(UartPort* _uartPort, uint16_t nodeAddress, bool isBroadcast)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {1500, 3000, 3000};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;
        const unsigned char length = 6;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(DEL_DEVICES);
        frame.append((nodeAddress >> 8) & 0xFF);
        frame.append(nodeAddress & 0xFF);
        frame.append(isBroadcast ? 0x01 : 0x00);

        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del DEL_DEVICE";
    }
}


void sendUartAddGroupManual(UartPort* _uartPort, uint16_t* address)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {1500, 2500, 3500};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;

        qDebug() << "UART GROUP SEND";
        unsigned char length = 9;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(ADD_GROUP_MANUAL);
        frame.append((address[0] >> 8) & 0xFF);
        frame.append(address[0] & 0xFF);
        frame.append((address[1] >> 8) & 0xFF);
        frame.append(address[1] & 0xFF);
        frame.append((address[2] >> 8) & 0xFF);
        frame.append(address[2] & 0xFF);

        qDebug() << address[0] << address[1] << address[2];

        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del ADD_GROUP_MANUAL";
    }
}

void sendUartAddGroup(UartPort* _uartPort, uint16_t* address)
{
    QByteArray frame;

    qDebug() << "UART GROUP SEND";
    unsigned char length = 9;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(ADD_GROUP);
    frame.append((address[0] >> 8) & 0xFF);
    frame.append(address[0] & 0xFF);
    frame.append((address[1] >> 8) & 0xFF);
    frame.append(address[1] & 0xFF);
    frame.append((address[2] >> 8) & 0xFF);
    frame.append(address[2] & 0xFF);

    qDebug() << address[0] << address[1] << address[2];

    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartDelGroup(UartPort* _uartPort, uint16_t* address, Database* database)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {1500, 2500, 3500};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;
        unsigned char length = 7;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(DEL_GROUP);
        frame.append((address[0] >> 8) & 0xFF);
        frame.append(address[0] & 0xFF);
        frame.append((address[1] >> 8) & 0xFF);
        frame.append(address[1] & 0xFF);
        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del DEL_GROUP";
    }
    else {
        for (uint8_t i = 0; i < MAX_SUBNET; i++) {
            for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
                if (meshDevice[i][j].getRealAddress() == address[0]) {
                    // Log entry
                    QString groupAddressString = QString("%1").arg(address[1], 4, 16, QLatin1Char('0')).toUpper();
                    QString name = "SUB:" + QString::number(i) + " " + "ID:" + QString::number(j) + " - " + database->getGroupName(groupAddressString);
                    QString serialNum = meshDevice[i][j].serialNumberString();
                    int btAddress = meshDevice[i][j].getRealAddress();
                    AntennaInfo info = getAntennaInfo(database);
                    QString eventType = "Groups";
                    insertLogEvent(database, name, serialNum, btAddress, info.ip, info.timestamp, LOG_DEL_FROM_GROUP, eventType);

                    meshDevice[i][j].delGroupSubAddress(address[1]);
                    database->delGroup(address[0], address[1]);
                    return;
                }
            }
        }
    }
}

void sendUartDelGroupSimple(UartPort* _uartPort, uint16_t* address)
{
    QByteArray frame;
    unsigned char length = 7;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(DEL_GROUP);
    frame.append((address[0] >> 8) & 0xFF);
    frame.append(address[0] & 0xFF);
    frame.append((address[1] >> 8) & 0xFF);
    frame.append(address[1] & 0xFF);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartDelGroupForAllNodes(UartPort* _uartPort, uint16_t groupAddress, Database* database)
{
    bool hasEnteredInSubnet;

    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        hasEnteredInSubnet = false;
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (meshDevice[i][j].getIsConfigured() && meshDevice[i][j].delGroupSubAddress(groupAddress)) {
                hasEnteredInSubnet = true;

                uint16_t nodeRealAddress = meshDevice[i][j].getRealAddress();

                sendUartInyectNode(_uartPort, nodeRealAddress, database);
                while(messageState == PENDING) {}

                if(messageState == RECEIVED) {
                    sendUartDelGroupForAllNodesUnitary(_uartPort, nodeRealAddress, groupAddress);
                    while(messageState == PENDING) {}

                    if(messageState == RECEIVED) {
                        database->delGroup(nodeRealAddress, groupAddress);
                    }
                }
            }
        }

        if(hasEnteredInSubnet) {
            sendUartClearInyectedNodes(_uartPort, false, database);
            while(messageState == PENDING) {}
        }
    }
}

void sendUartDelGroupForAllNodesUnitary(UartPort* _uartPort, uint16_t nodeRealAddress, uint16_t groupAddress)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {1500, 2500, 3500};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;
        unsigned char length = 7;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(DEL_GROUP);
        frame.append((nodeRealAddress >> 8) & 0xFF);
        frame.append(nodeRealAddress & 0xFF);
        frame.append((groupAddress >> 8) & 0xFF);
        frame.append(groupAddress & 0xFF);
        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del DEL_GROUP_FOR_ALL_NODES_UNITARY";
    }
}

void sendUartClearAllData(UartPort* _uartPort, uint16_t nodeAddress)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {1000, 2000, 2500};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;
        unsigned char length = 5;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(CLEAR_ALL_DATA);
        frame.append((nodeAddress >> 8) & 0xFF);
        frame.append(nodeAddress & 0xFF);

        frame.append(UART_END);

        qDebug() << "Enviando borrado a nodo:" << QString::asprintf("%04X", nodeAddress);
        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del CLEAR_ALL_DATA";
    }


}

void sendUartStartLineScanning(UartPort* _uartPort)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {1000, 2000, 3000};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;

        unsigned char length = 3;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(START_LINE_SCANNING);
        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del START_LINE_SCANNING";
    }
}

void sendUartLineScanning(UartPort* _uartPort, uint8_t phase, uint16_t nodeAddress)
{
    QByteArray frame;

    unsigned char length = 6;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(LINE_SCANNING);
    frame.append(phase);
    frame.append((nodeAddress >> 8) & 0xFF);
    frame.append(nodeAddress & 0xFF);

    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartEndLineScanning(UartPort* _uartPort)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {1000, 2000, 4000};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;

        unsigned char length = 3;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(END_LINE_SCANNING);
        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del END_LINE_SCANNING";
    }
}

void sendUartChangeFather(UartPort* _uartPort, uint16_t childRealAddress, uint16_t fatherRealAddress)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {500, 1500, 2500};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;
        unsigned char length = 7;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(CHANGE_FATHER);
        frame.append((childRealAddress >> 8) & 0xFF);
        frame.append(childRealAddress & 0xFF);
        frame.append((fatherRealAddress >> 8) & 0xFF);
        frame.append(fatherRealAddress & 0xFF);
        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del CHANGE_FATHER";
    }
}

void sendUartDaliCommand(UartPort* _uartPort, uint16_t targetAddress, uint8_t daliRegister1, uint8_t daliRegister2, uint8_t commandType)
{
    QByteArray frame;
    unsigned char length = 7;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_DALI_FRAME_TYPE);
    frame.append((targetAddress >> 8) & 0xFF);
    frame.append(targetAddress & 0xFF);
    frame.append(daliRegister1);
    frame.append(daliRegister2);
    frame.append(commandType);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}


void sendPollingFrame(UartPort* _uartPort, uint16_t nodeAddress)
{
    if(isCommissioning || isManualAddingDevice || isScanning || isLineScanning || isReplacingDevices || isClearingAllData) { return; }
    if(nodeAddress == 0x0000) { return; }

    QByteArray frame;
    unsigned char length = 4;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_POLLING_FRAME_TYPE);
    frame.append((nodeAddress >> 8) & 0xFF);
    frame.append(nodeAddress & 0xFF);
    frame.append(UART_END);

    qDebug() << "ENVIANDO POLLING " << nodeAddress;

    _uartPort->sendData(frame);
}

void sendWriteIDCodeFrame(UartPort* _uartPort, QString factoryCode)
{
    bool ok;
    uint8_t code[4] = {0};
    QStringList factoryCodeParts = factoryCode.split(".");

    for (uint8_t i = 0; i < factoryCodeParts.size(); i++) { code[i] = factoryCodeParts[i].toInt(&ok, 16); }
    QByteArray frame;
    unsigned char length = 7;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(WRITE_ID_CODE);
    frame.append(code[0]);
    frame.append(code[1]);
    frame.append(code[2]);
    frame.append(code[3]);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartPOLForUpdate(UartPort* _uartPort, Database* database)
{
    // Se genera un SET con todas las direcciones de los grupos existentes (los 4 por defecto y los creados manualmente)
    QSet<QString> groupsSet = {"C000", "C001", "C002", "C003"};

    QList<QPair<QString, QString>> groups = database->getGroups();
    for(QPair<QString, QString> pair : groups)
        groupsSet.insert(pair.first);

    // Se recorre la estructura de devices para cruzar los grupos y los nodos
    // Se crea una estructura de pares tal que el primer elemento es un nodo y el segundo es una lista con las addresses de los grupos donde ese nodo es el primero con comunicación
    crossedGroupAndNodes.clear();
    for(int i = 0; i < MAX_SUBNET; i++){
        for(int j = 0; j < MAX_NODES_SUBNET; j++) {
            Device& device = meshDevice[i][j];
            if(device.getIsConfigured() && !device.hasCommunicationFailure()) {
                qDebug() << "Device" << QString::number(i * 64 + j + 1);
                for(const QString &groupAddress : QSet<QString>(groupsSet)) {
                    if(device.isOnGroupSubAddress(groupAddress.toUShort(nullptr, 16))) {
                        qDebug() << "MATCHING WITH" << groupAddress;

                        bool existingEntry = false;

                        for(QPair<uint16_t, QStringList> &par : crossedGroupAndNodes) {
                            if(par.first == device.getRealAddress()) {
                                par.second.append(groupAddress);
                                existingEntry = true;
                                break;
                            }
                        }

                        if(!existingEntry) {
                            crossedGroupAndNodes.append(QPair<uint16_t, QStringList>(device.getRealAddress(), {groupAddress}));
                        }

                        groupsSet.remove(groupAddress);
                    }
                }
            }
        }
    }

    // Para probar la creación correcta de la estructura que cruza (nodos) - (grupos en los que el nodo es el primero con comunicación)
    for(int i = 0; i < crossedGroupAndNodes.size(); i++) {
        qDebug() << "[" << i << "] -" << crossedGroupAndNodes[i].first << "-" << crossedGroupAndNodes[i].second;
    }

    for (int i = 0; i < crossedGroupAndNodes.size(); i++) {
        uint16_t realAddress = crossedGroupAndNodes[i].first;

        sendUartInyectNode(_uartPort, realAddress, database);
        while(messageState == PENDING) {}

        if(messageState == RECEIVED) {
            sendUartPOLForUpdateUnitary(_uartPort, realAddress);

            //delay(SLEEP_DALI_TIME_MS * 2);
            while(messageState == PENDING) {}
        }
    }

    delay(1000);
    sendUartClearInyectedNodes(_uartPort, false, database);
    while(messageState == PENDING) {}
}

void sendUartPOLForUpdateUnitary(UartPort* _uartPort, uint16_t realAddress)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {750, 1500, 2500};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;
        unsigned char length = 5;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(ASK_POWER_ON_LEVEL);
        frame.append((realAddress >> 8) & 0xFF);
        frame.append(realAddress & 0xFF);
        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del POL_FOR_UPDATE_UNITARY";
    }
}

void sendUartSetRelay(UartPort* _uartPort, uint16_t nodeAddress, bool enable)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {2000, 2000, 2000};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;
        unsigned char length = 6;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(SET_RELAY);
        frame.append((nodeAddress >> 8) & 0xFF);
        frame.append(nodeAddress & 0xFF);
        frame.append(enable ? 0x01 : 0x00);
        frame.append(UART_END);

        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del SET_RELAY";
    }
}

void sendUartScanFromNode(UartPort* _uartPort, uint16_t nodeRealAddress)
{
    uint8_t att = 3;
    uint8_t actAtt = 0;
    int ms[3] = {500, 2000, 2500};
    messageState = PENDING;

    while(actAtt < att && messageState == PENDING) {
        QByteArray frame;
        unsigned char length = 5;

        frame.append(UART_HEADER);
        frame.append(length);
        frame.append(UART_CONFIG_FRAME_TYPE);
        frame.append(SCAN_FROM_NODE);
        frame.append((nodeRealAddress >> 8) & 0xFF);
        frame.append(nodeRealAddress & 0xFF);
        frame.append(UART_END);

        qDebug() << "Enviando escaneo desde nodo:" << QString::asprintf("%04X", nodeRealAddress);
        _uartPort->sendData(frame);

        delay(ms[actAtt]);
        actAtt++;
    }

    if(messageState == PENDING) {
        messageState = MISSED;
        qDebug() << "No se recibió confirmación del SCAN_FROM_NODE";
    }
}

void sendNetKey(UartPort* _uartPort, Database* database)
{
    QString netKey = database->getNetKey();
    uint8_t netKeyBytes[16];

    if(netKey.size() == 32) {
        for(uint8_t i = 0; i < 16; i++) {
            QString byteString = netKey.mid(i * 2, 2);
            netKeyBytes[i] = static_cast<uint8_t>(byteString.toUInt(nullptr, 16));
        }
    } else {
        memcpy(netKeyBytes, netKeys[netKey.toInt() - 1], 16);
    }

    QByteArray frame;
    unsigned char length = 19;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(SEND_NET_KEY);
    for(uint8_t j = 0; j < 16; j++) {
        frame.append(netKeyBytes[j]);
    }
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendAntennaAddress(UartPort* _uartPort, Database* database)
{
    uint16_t masterStoredAddress = database->getMasterRealAddress();

    QByteArray frame;
    unsigned char length = 5;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(SEND_ANTENNA_ADDRESS);
    frame.append((masterStoredAddress >> 8) & 0xFF);
    frame.append(masterStoredAddress & 0xFF);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendAntennaGetAddress(UartPort* _uartPort)
{
    QByteArray frame;
    unsigned char length = 3;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(GET_ANTENNA_ADDRESS);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendAntennaAddressAndNetKey(UartPort* _uartPort, bool antennaIDHasChanged, bool netKeyHasChanged)
{
    QByteArray frame;
    unsigned char length = 5;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(ADDRESS_AND_NET_KEY);
    frame.append(antennaIDHasChanged ? 0x01 : 0x00);
    frame.append(netKeyHasChanged ? 0x01 : 0x00);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartConfirmReplacing(UartPort* _uartPort, uint16_t realAddress)
{
    QByteArray frame;
    unsigned char length = 5;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(CONFIRM_REPLACE);
    frame.append((realAddress >> 8) & 0xFF);
    frame.append(realAddress & 0xFF);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}
