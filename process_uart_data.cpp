#include "process_uart_data.h"
#include "process_webserver_data.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "time_functions.h"
#include "global_variables.h"

QByteArray uartBuffer;
static bool secondBufferRequired = false;

static bool checkCRC(QByteArray data)
{
    uint8_t checkSum, crc = 0;

    for (uint8_t i = 1; i < (data.length() - 1); i++) { crc += (unsigned char)data[i]; }

    checkSum = (unsigned char)data[data.length() - 1];

    if (crc == checkSum) { return true; }
    else { return false; }
}

static QByteArray processUartFrame(QByteArray data)
{
    if (!secondBufferRequired) {
        uartBuffer.clear();

        for (uint8_t i = 0; i < data.size(); i++) { uartBuffer.append(data[i]); }

        if (data.size() > 1) {
            if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == SCAN_DEVICES) {
                if (data.size() != 22) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == CONFIRM_START_COMMISSION) {
                if (data.size() != 4) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == CONFIRM_ADD_DEVICE) {
                if (data.size() != 4) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == CONFIRM_GROUP_FRAME) {
                if (data.size() != 4) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == CONFIRM_NEW_ITERATION) {
                if (data.size() != 4) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == CONFIRM_CHANGE_RELAY) {
                if (data.size() != 4) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == ADD_DEVICE) {
                if (data.size() != 6) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == DEVICE_ERROR) {
                if (data.size() != 20) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == FEATURES) {
                if (data.size() != 26) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == GROUP_ADDED) {
                if (data.size() != 10) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CONFIG_FRAME_TYPE && (unsigned char)data[2] == DEBUG) {
                if (data.size() != 5) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_CHANGE_FRAME_TYPE) {
                if (data.size() != 7) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_RSP_POLLING_FRAME_TYPE) {
                if (data.size() != 9) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_ID_FRAME_TYPE && (unsigned char)data[2] == FACTORY_ID_WROTE) {
                if (data.size() != 4) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_ID_FRAME_TYPE && (unsigned char)data[2] == DALI_TESTED) {
                if (data.size() != 4) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
            else if ((unsigned char)data[0] == UART_HEADER && (unsigned char)data[1] == UART_ID_FRAME_TYPE && (unsigned char)data[2] == RECORDED_DEVICE) {
                if (data.size() != 4) {
                    secondBufferRequired = true;
                    return QByteArray();
                }
            }
        }
        else {
            secondBufferRequired = true;
            return QByteArray();
        }
    }
    else {
        for (uint8_t i = 0; i < data.size(); i++) { uartBuffer.append(data[i]); }

        secondBufferRequired = false;
    }

    return uartBuffer;
}

static bool addDeviceFrameReceived = false;
static bool featuresFrameReceived = false;

void processUartData(QByteArray data, WebServer* webServer, UartPort* uartPort, Database* database)
{
    QString hexString;
    for (uint8_t i = 0; i < data.size(); i++) {
        hexString += QString::asprintf("%02X ", static_cast<unsigned char>(data[i]));
    }
    qDebug() << hexString.trimmed();
    //qDebug() << "DATA SIZE" << data.size();
    QByteArray dataChecked = processUartFrame(data);
    if (!dataChecked.isEmpty() && (unsigned char)dataChecked[0] == UART_HEADER && checkCRC(data)) {
        switch ((unsigned char)dataChecked[1]) {
            case UART_RSP_CONFIG_FRAME_TYPE:
                switch ((unsigned char)dataChecked[2]) {
                    case SCAN_DEVICES:
                        sendScannedDevices(dataChecked, webServer);
                    break;

                    case CONFIRM_START_COMMISSION:
                        //qDebug() << "PRUEBA UART";
                        qDebug() << "PARANDO TIMER START COMMISSION";
                        addDeviceTimer.start(ADD_DEVICE_TIMER_MS);
                        sendConfirmStartCommission(webServer);
                    break;

                    case CONFIRM_ADD_DEVICE:
                        qDebug() << "PARANDO TIMER ADD DEVICE";
                        confirmAddDeviceTimer.stop();
                        //sendConfirmAddingDevice(webServer);
                    break;

                    case CONFIRM_GROUP_FRAME:
                        qDebug() << "PARANDO TIMER GROUP FRAME";
                        groupFrameTimer.stop();
                    break;

                    case CONFIRM_NEW_ITERATION:
                        //commissionData.numberOfNodesScanned--;
                        qDebug() << "PARANDO TIMER NEW ITERATION";
                        newIterationTimer.stop();
                        numberOfIterations--;
                        addDeviceTimer.start(ADD_DEVICE_TIMER_MS);
                    break;

                    case CONFIRM_CHANGE_RELAY:
                        qDebug() << "CONFIRM CHANGE RELAY";
                        commissionData.isChangeRelayConfirmed = true;
                    break;

                    case ADD_DEVICE:
                        sendAddedDevices(dataChecked, webServer, database);
                    break;

                    case DEVICE_ERROR:
                        qDebug() << "DEVICE ERROR";
                        sendDeviceError(dataChecked, uartPort, webServer);
                    break;

                    case FEATURES:
                        processFeaturesFrame(dataChecked, uartPort, database, webServer);
                    break;

                    case GROUP_ADDED:
                        processGroupAddedFrame(dataChecked, uartPort, database, webServer);
                    break;

                    case DEBUG:
                        qDebug() << "DEBUG FRAME:" << QString("0x%1").arg((unsigned char)dataChecked[3], 2, 16, QChar('0')).toUpper();
                    break;

                    default:
                    break;
                }
            break;

            case UART_RSP_CHANGE_FRAME_TYPE:
                processChangeFrame(dataChecked, database, webServer);
            break;

            case UART_RSP_POLLING_FRAME_TYPE:
                processPollingFrame(dataChecked);
            break;

            case UART_ID_FRAME_TYPE:
                switch ((unsigned char)dataChecked[2]) {
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
    uint8_t deviceType, ratedDuration, emergencyFeatures, physicalMinLvl;
    uint8_t nodeUUID[16];
    uint16_t address;

    address = ((unsigned char)data[3] << 8) + (unsigned char)data[4];
    QString value = "";

    for (uint8_t i = 0; i < 16; i++) {
        nodeUUID[i] = (unsigned char)data[5 + i];
        value += QString::asprintf("%02X", nodeUUID[i]);
    }

    deviceType = (unsigned char)data[21];
    ratedDuration = (unsigned char)data[22];
    emergencyFeatures = (unsigned char)data[23];
    physicalMinLvl = (unsigned char)data[24];


    qDebug() << "FEATURES" << value << "FRAME:" << address << deviceType << ratedDuration << emergencyFeatures << physicalMinLvl;

    commissionData.numberOfNodesAdded++;
    numberOfIterations++;

    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (!meshDevice[i][j].getIsConfigured()) {
                meshDevice[i][j].setRealAddress(address);
                meshDevice[i][j].setSerialNumber(nodeUUID);
                meshDevice[i][j].setDeviceType(deviceType);
                meshDevice[i][j].setRatedDuration(ratedDuration);
                meshDevice[i][j].setEmergencyFeatures(emergencyFeatures);
                meshDevice[i][j].setPhysicalMinLvl(physicalMinLvl);
                meshDevice[i][j].setIsConfigured(true);

                netAddress = i * 64 + j + 1;

                database->setNewNode(i, j, address, nodeUUID);

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

                database->setNodeFeatures(address, deviceType, ratedDuration, emergencyFeatures, physicalMinLvl);

                /*
                QString message = QString(WS_SEND_ADDED_DEVICES) + "@" + QString::number(i * 64 + j + 1);
                if (webServer != nullptr) { webServer->sendData(message); }
                //*/

                qDebug()  << "NODO AÑADIDO";

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
                    timerGroupAddress[2] = 0x0000;
                }
                groupFrameTimer.start(GROUP_FRAME_TIMER_MS);
                sendUartAddGroup(uartPort, timerGroupAddress);
                return;
                //*/
            }
        }
    }
}

void processGroupAddedFrame(QByteArray data, UartPort* uartPort, Database* database, WebServer* webServer)
{
    uint16_t nodeAddress, deviceTypeGroupAddress, netAdressGroupAddress;

    nodeAddress = ((unsigned char)data[3] << 8) + (unsigned char)data[4];
    deviceTypeGroupAddress = ((unsigned char)data[5] << 8) + (unsigned char)data[6];
    netAdressGroupAddress = ((unsigned char)data[7] << 8) + (unsigned char)data[8];

    qDebug() << "GROUP ADDED FRAME: " << nodeAddress << deviceTypeGroupAddress << netAdressGroupAddress;
    database->setGroup(nodeAddress, deviceTypeGroupAddress);
    if (netAdressGroupAddress != 0x0000) { database->setGroup(nodeAddress, netAdressGroupAddress); }

    QString message = QString(WS_SEND_ADDED_DEVICES) + "@" + QString::number(netAddress);
    if (webServer != nullptr) { webServer->sendData(message); }

    netAddress = 0;

    qDebug()  << "NODO AÑADIDO";

    delay(4000);

    uint8_t emptyUUID[16] = {0};
    for (uint8_t i = 0; i < 20; i++) {
        if (commissionData.numberOfNodesScanned == commissionData.numberOfNodesAdded) {
            commissionData.numberOfNodesScanned = 0;
            commissionData.numberOfNodesAdded = 0;
            sendUartNewIteration(uartPort);
            newIterationTimer.start(NEW_ITERATION_TIMER_MS);
            break;
        }
        if (memcmp(scannedUUID[i].UUID, emptyUUID, sizeof(emptyUUID)) != 0) {
            qDebug() << "ADDING NEW NODE UUID" << QString("0x%1").arg(scannedUUID[i].UUID[0], 2, 16, QChar('0')).toUpper();
            sendUartAddDevice(uartPort, scannedUUID[i]);
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

void sendUartStartCommission(UartPort* _uartPort)
{
    QByteArray frame;
    unsigned char length = 3;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(START_COMMISSION);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartNewIteration(UartPort* _uartPort)
{
    QByteArray frame;
    unsigned char length = 3;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(NEW_ITERATION);
    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartChangeRelay(UartPort* _uartPort, uint16_t nodeAddress)
{
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
    QByteArray frame;
    unsigned char length = 21;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(ADD_DEVICE);
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

void sendUartDelDevice(UartPort* _uartPort, uint16_t nodeAddress)
{
    QByteArray frame;
    unsigned char length = 5;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(DEL_DEVICE);
    frame.append((nodeAddress >> 8) & 0xFF);
    frame.append(nodeAddress & 0xFF);
    frame.append(UART_END);

    _uartPort->sendData(frame);
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

    qDebug() << address[0] << address[1] << address[1];

    frame.append(UART_END);

    _uartPort->sendData(frame);
}

void sendUartDelGroup(UartPort* _uartPort, uint16_t* address, Database* database)
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

    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (meshDevice[i][j].getRealAddress() == address[0]) {
                meshDevice[i][j].delGroupSubAddress(address[1]);
                database->delGroup(address[0], address[1]);
                return;
            }
        }
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
