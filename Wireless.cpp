#include "Wireless.h"
#include "time_functions.h"
#include "file_handler.h"
#include "process_eth_data.h"
#include "process_webserver_data.h"
#include "process_uart_data.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "log.h"
#include "embedded_io.h"
#include <QSqlQuery>
#include <QDateTime>
#include "file_handler.h"

Wireless::Wireless(QObject *parent)
    : QObject{parent}
{
    _udpSocket = new UdpSocket(this);
    _uartPort = new UartPort(this);
    _webServer = new WebServer(this);
    _database = new Database(this);

    pollingTimer.setInterval(POLLING_TIMER_MS);
    pollingTimer.stop();
    testTimer.setInterval(TEST_TIMER_MS);
    testTimer.start();
    connect(_udpSocket, &UdpSocket::dataReceived, this, &Wireless::udpReceivedData);
    connect(_uartPort, &UartPort::dataReceived, this, &Wireless::uartReceivedData);
    connect(_webServer, &WebServer::messageReceived, this, &Wireless::webServerReceivedData);
    connect(&pollingTimer, &QTimer::timeout, this, &Wireless::pollingTimerHandler);
    connect(&testTimer, &QTimer::timeout, this, &Wireless::testTimerHandler);
    connect(&groupFrameTimer, &QTimer::timeout, this, &Wireless::groupFrameTimerHandler);
    connect(&addDeviceTimer, &QTimer::timeout, this, &Wireless::addDeviceTimerHandler);
    connect(&confirmAddDeviceTimer, &QTimer::timeout, this, &Wireless::confirmAddDeviceTimerHandler);
    connect(&newIterationTimer, &QTimer::timeout, this, &Wireless::newIterationTimerHandler);
    connect(&replaceP2Timer, &QTimer::timeout, this, &Wireless::replaceP2TimerHandler);
    replaceP2Timer.setSingleShot(true);
    connect(&replaceP3Timer, &QTimer::timeout, this, &Wireless::replaceP3TimerHandler);
    replaceP3Timer.setSingleShot(true);
    connect(&cleanCdbTimer, &QTimer::timeout, this, &Wireless::cleanCdbTimerHandler);
    connect(&askInitDataFromMicroTimer, &QTimer::timeout, this, &Wireless::askInitDataFromMicroTimerHandler);
    askInitDataFromMicroTimer.setSingleShot(true);
    connect(&answerFactoryProgramTimer, &QTimer::timeout, this, &Wireless::answerFactoryProgramTimerHandler);
    answerFactoryProgramTimer.setSingleShot(true);
    connect(&testResultCheckTimer, SIGNAL(timeout()), this, SLOT(checkTestResultsHandler()));
    testResultCheckTimer.start(LOG_DATA_TIME_MS);
    identifyTimer.setInterval(IDENTIFY_TIMER_MS);
    identifyTimer.setSingleShot(false);
    connect(&identifyTimer, &QTimer::timeout, this, &Wireless::identifyTimerHandler);

}

void Wireless::setEmbeddedIO(EmbeddedIO* io) {
    _io = io;
    QObject::connect(_io, SIGNAL(testButtonPressed()),
                     this, SLOT(onTestButtonPressed()));

    //qDebug() << "[TEST] recibido en Wireless::onTestButtonPressed()";

}


void Wireless::runNetwork()
{
    setWebServerData(_database);
    getMacAddress();

    for (uint8_t i = 0; i < 20; i++) {
        memset(scannedUUID[i].UUID, 0, sizeof(scannedUUID[i].UUID));
        scannedUUID[i].nodeAddressReport = 0;
    }

    commissionData.numberOfNodesScanned = 0;
    commissionData.numberOfNodesAdded = 0;
    commissionData.isRelayNode = false;

    _database->loadNodesFromDatabase();
    _database->loadTestsFromDatabase();

    _database->loadFailComCycles();

    while(notRan) {
        sendSetAntennaAddressAndInstallKey(_uartPort, _database);
        while(messageState == PENDING) {}

        if(messageState == RECEIVED) {
            sendGetAntennaAddressAndInstallKey(_uartPort);
            while(messageState == PENDING) {}

            if(messageState == RECEIVED) {
                delay(300);
                notRan = false;
                qDebug() << "EXITO!!! La antena se inició correctamente.";
            }
            else {
                qDebug() << "ERROR!!! El micro no respondió [correctamente] al segundo mensaje de inicio [GET].";
                sendInitAlert(_webServer);
                delay(1000);
            }
        }
        else {
            qDebug() << "ERROR!!! El micro no respondió [correctamente] al primer mensaje de inicio [SET].";
            sendInitAlert(_webServer);
            delay(1000);
        }
    }

    pollingTimer.start(POLLING_TIMER_MS);
/*
    QByteArray data;
    data.append(0x01);
    data.append(0x23);
    data.append(0x45);
    data.append(0x67);
    data.append(0x89);

    QHostAddress dstAddress;
    dstAddress.setAddress("192.168.1.111");

    while(1) {
        _uartPort->sendData(data);
        delay(1000);
    }
//*/
}

void Wireless::udpReceivedData(QQueue <QPair <QString, QByteArray> >* rcvData)
{
    if (rcvData->isEmpty()) { return; }
    QPair <QString, QByteArray> data = rcvData->dequeue();
    QString rcvAddress = data.first.remove("::ffff:");
    QByteArray dataBuffer = data.second;

    if (checkFrameHeader(dataBuffer) && checkCRC(dataBuffer) && checkRcvAddress(rcvAddress)) {
        processEthFrame(rcvAddress, dataBuffer, _udpSocket, _database, _uartPort);
    }
    else if(dataBuffer.startsWith("NORMALINK-G")) {
        rcvAddressFactoryProgram = rcvAddress;
        processFactoryProgramSerial(_uartPort, dataBuffer);
    }
}

void Wireless::uartReceivedData(QByteArray data)
{
    //processUartData(data, _webServer, _uartPort, _database);
    extractAndProcessFrames(data, _webServer, _uartPort, _database);
}

void Wireless::webServerReceivedData(QString data)
{
    processWebServerData(data, _webServer, _uartPort, _database);
}

void Wireless::pollingTimerHandler()
{
    if (pollingData.pollingInProgress) {

        Device &dev = meshDevice[subnetCount][nodeSubnetCount];

        if (pollingData.pollingReceived) {
            // Reseteamos tanto el retry como los ciclos de fallo
            pollingData.pollingReceived = false;
            pollingData.pollingInProgress = false;
            pollingData.retries = 0;
            dev.resetCommunicationFailure();
            nodeSubnetCount++;
            goto sendNewPolling;
        }
        else if (pollingData.retries < 5) {
            // Reintentos dentro del mismo ciclo
            pollingData.retries++;
            sendPollingFrame(_uartPort, dev.getRealAddress());
            return;
        }
        else {
            // Se han agotado los 5 reintentos de este ciclo
            pollingData.retries = 0;
            pollingData.pollingInProgress = false;
            dev.registerCommunicationFailureCycle();
            nodeSubnetCount++;
        }
    }

sendNewPolling:
    while (subnetCount < MAX_SUBNET) {
        while (nodeSubnetCount < MAX_NODES_SUBNET) {
            Device &device = meshDevice[subnetCount][nodeSubnetCount];
            if (device.getIsConfigured()) {
                updateLogsByPollings(device);
                sendPollingFrame(_uartPort, device.getRealAddress());
                pollingData.pollingInProgress = true;
                return;
            }
            nodeSubnetCount++;
        }
        nodeSubnetCount = 0;
        subnetCount++;
    }

    // Cuando terminamos una vuelta completa a la red:
    nodeSubnetCount = 0;
    subnetCount = 0;
}

void Wireless::updateLogsByPollings(Device &device)
{
    bool lampNow = device.hasLampFailure();
    bool lampPrev = device.getPrevLampFail();
    bool batNow = device.hasBatteryFailure();
    bool batPrev = device.getPrevBatteryFail();

    bool durNow = device.hasBatteryDurationFailure();
    bool durPrev = device.getPrevDurationFail();
    bool commNow = device.hasCommunicationFailure();
    bool commPrev = device.getPrevCommFail();

    int globalPos = subnetCount * 64 + nodeSubnetCount + 1;
    QString name = "A" + QString::number(globalPos).rightJustified(4, '0');
    QString serialNum = device.serialNumberString();
    int btAddress = device.getRealAddress();
    AntennaInfo info = getAntennaInfo(_database);
    QString eventType = "Fail";

    if (lampNow != lampPrev) {
        if (lampNow) {
            insertLogEvent(_database, name, serialNum, btAddress, info.ip, info.timestamp, LOG_LAMP_FAILURE, eventType);
        } else {
            insertLogEvent(_database, name, serialNum, btAddress, info.ip, info.timestamp, LOG_LAMP_RECOVERED, eventType);
        }
        device.setPrevLampFail(lampNow);
    }
    if (batNow != batPrev) {
        if (batNow) {
            insertLogEvent(_database, name, serialNum, btAddress, info.ip, info.timestamp, LOG_BATTERY_FAILURE, eventType);
        } else {
            insertLogEvent(_database, name, serialNum, btAddress,info.ip, info.timestamp, LOG_BATTERY_RECOVERED, eventType);
        }
        device.setPrevBatteryFail(batNow);
    }
    if (durNow != durPrev) {
        if (durNow) {
            insertLogEvent(_database, name, serialNum, btAddress, info.ip, info.timestamp, LOG_DURATION_FAILURE, eventType);
        } else {
            insertLogEvent(_database, name, serialNum, btAddress, info.ip, info.timestamp, LOG_DURATION_RECOVERED, eventType);
        }
        device.setPrevDurationFail(durNow);
    }
    if (commNow != commPrev) {
        if (commNow) {
            insertLogEvent(_database, name, serialNum, btAddress, info.ip, info.timestamp, LOG_COMMUNICATION_FAILURE, eventType);
        } else {
            insertLogEvent(_database, name, serialNum, btAddress, info.ip, info.timestamp, LOG_COMMUNICATION_RECOVERED, eventType);
        }
        device.setPrevCommFail(commNow);
    }
}

void Wireless::updateLogsByTests(uint8_t i, uint8_t code)
{
    QString name = _database->getGroupName(tests[i].getGroupAddress()) + " [G]";
    QString serailNum = "FF.FF.FF.FF";
    int btAddress = tests[i].getGroupAddress().toUInt(NULL, 16);
    QString eventType = "Test";
    AntennaInfo info = getAntennaInfo(_database);

    insertLogEvent(_database, name, serailNum, btAddress, info.ip, info.timestamp, code, eventType);

    AntennaTestCheck testCheck;
    testCheck.groupId = btAddress;
    testCheck.testType = code == LOG_TEST_REQUESTED_FUNCTIONAL ? "FUNCTIONAL" : "DURATION";
    testCheck.checkTime = info.timestamp.time().addSecs(LOG_TEST_REQUESTED_FUNCTIONAL ? 900 : 43200);

    antennaTestCheckList.append(testCheck);
}

void Wireless::testTimerHandler()
{
    QString date = getLocalDate();
    QString time = getLocalTime();
    QString dayName = getLocalDay();

    QStringList timeParts = time.split(":");
    QString timeHM = timeParts[0] + ":" + timeParts[1];

    for (uint8_t i = 0; i < MAX_TEST; i++) {
        if (tests[i].getFunctionalEnable()) {
            QStringList functionalDays = tests[i].getFunctionalDays().split(" ");
            for(const QString &functionalDay : functionalDays) {
                if (tests[i].getFunctionalTime() == timeHM &&  functionalDay == dayName) {
                    pollingTimer.stop();
                    sendUartDaliCommand(_uartPort, tests[i].getGroupAddress().toUInt(NULL, 16), ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
                    delay(SLEEP_DALI_TIME_MS);
                    sendUartDaliCommand(_uartPort, tests[i].getGroupAddress().toUInt(NULL, 16), BROADCAST_ADDR, START_FUNCTION_TEST, IS_TWICE);
                    delay(SLEEP_DALI_TIME_MS);
                    pollingTimer.start(POLLING_TIMER_MS);

                    updateLogsByTests(i, LOG_TEST_REQUESTED_FUNCTIONAL);
                }
            }
        }
        if (tests[i].getDurationEnable()) {
            QString durationPeriodicity = tests[i].getDurationPeriodicity();
            uint8_t periodicityMonths = durationPeriodicity.toUInt();
            QDate startDate = QDate::fromString(tests[i].getDurationDate(), "yyyy-MM-dd");
            QDate currentDate = QDate::fromString(date, "yyyy-MM-dd");

            if (tests[i].getDurationTime() == timeHM && startDate == currentDate) {
                pollingTimer.stop();
                sendUartDaliCommand(_uartPort, tests[i].getGroupAddress().toUInt(NULL, 16), ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
                delay(SLEEP_DALI_TIME_MS);
                sendUartDaliCommand(_uartPort, tests[i].getGroupAddress().toUInt(NULL, 16), BROADCAST_ADDR, START_DURATION_TEST, IS_TWICE);
                pollingTimer.start(POLLING_TIMER_MS);

                updateLogsByTests(i, LOG_TEST_REQUESTED_DURATION);
            }
            else {
                QDate nextTestDate = startDate;
                while (nextTestDate <= currentDate) {
                    if (tests[i].getDurationTime() == timeHM && nextTestDate == currentDate) {
                        pollingTimer.stop();
                        sendUartDaliCommand(_uartPort, tests[i].getGroupAddress().toUInt(NULL, 16), ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
                        delay(SLEEP_DALI_TIME_MS);
                        sendUartDaliCommand(_uartPort, tests[i].getGroupAddress().toUInt(NULL, 16), BROADCAST_ADDR, START_DURATION_TEST, IS_TWICE);
                        pollingTimer.start(POLLING_TIMER_MS);

                        updateLogsByTests(i, LOG_TEST_REQUESTED_DURATION);

                        break;
                    }
                    nextTestDate = nextTestDate.addMonths(periodicityMonths);
                }
            }
        }
    }
}

void Wireless::checkTestResultsHandler()
{
    QString timeNowStr = getLocalTime(); // format: "HH:mm:ss"
    QTime timeNow = QTime::fromString(timeNowStr, "HH:mm:ss");

    for (int i = antennaTestCheckList.size() - 1; i >= 0; i--) {
        const AntennaTestCheck &check = antennaTestCheckList[i];

        if (check.checkTime.hour() == timeNow.hour() &&
            check.checkTime.minute() == timeNow.minute()) {
            for (int subnet = 0; subnet < MAX_SUBNET; subnet++) {
                for (int node = 0; node < MAX_NODES_SUBNET; node++) {
                    Device &device = meshDevice[subnet][node];

                    if (!device.getIsConfigured()){ continue; }

                    uint16_t realAddress = device.getRealAddress();
                    bool shouldCheckThisDevice = false;

                    if (check.groupId < 0xC000) {
                        // Direct device test
                        shouldCheckThisDevice = (realAddress == check.groupId);

                    } else {
                        // Group test
                        shouldCheckThisDevice = _database->deviceIsInGroup(realAddress, check.groupId);
                    }

                    if (!shouldCheckThisDevice){ continue; }

                    bool com = device.hasCommunicationFailure();
                    bool bat = device.hasBatteryFailure();
                    bool lamp = device.hasLampFailure();
                    bool dur = device.hasBatteryDurationFailure();
                    bool failed = com || bat || lamp || dur;

                    QString eventType = "Test";
                    AntennaInfo info = getAntennaInfo(_database);
                    int globalPos = subnet * 64 + node + 1;
                    QString name = "A" + QString::number(globalPos).rightJustified(4, '0');
                    QString serial = device.serialNumberString();
                    int btAddress = device.getRealAddress();

                    if (check.testType == "FUNCTIONAL") {
                        insertLogEvent(_database, name, serial, btAddress, info.ip, info.timestamp, LOG_TEST_COMPLETED_FUNCTIONAL, eventType);
                        insertLogEvent(_database, name, serial, btAddress, info.ip, info.timestamp,
                                       failed ? LOG_TEST_FT_FAIL : LOG_TEST_FT_OK, eventType);
                    } else {
                        insertLogEvent(_database, name, serial, btAddress, info.ip, info.timestamp, LOG_TEST_COMPLETED_DURATION, eventType);
                        insertLogEvent(_database, name, serial, btAddress, info.ip, info.timestamp,
                                       failed ? LOG_TEST_DT_FAIL : LOG_TEST_DT_OK, eventType);
                    }
                }
            }
            // Remove test check entry
            antennaTestCheckList.removeAt(i);
        }
    }
}

void Wireless::groupFrameTimerHandler()
{
    qDebug() << "GROUP FRAME TIMER";
    sendUartAddGroup(_uartPort, timerGroupAddress);
}

void Wireless::addDeviceTimerHandler()
{
    addDeviceTimer.stop();
    //sendStartAddingDevices(_webServer);

    if (commissionData.isRelayNode && scannedUUID[0].nodeAddressReport != antennaRealAddress) {
        if (numberOfIterations != 0 && !forceStopCommissioning) {
            do {
                qDebug() << "CHANGE RELAY NODE" << scannedUUID[0].nodeAddressReport;
                sendUartChangeRelay(_uartPort, scannedUUID[0].nodeAddressReport,_database);
                delay(5000);
            } while (!commissionData.isChangeRelayConfirmed);
        }
    }

    qDebug() << "IS CHANGE FALSE";
    commissionData.isChangeRelayConfirmed = false;

    if (commissionData.numberOfNodesScanned > 0 && !forceStopCommissioning) {
        qDebug() << "EMPEZAMOS A AÑADIR NODOS";
        sendLogCommissionEntry(_webServer, "Start adding nodes...", "INFO");
        commissionData.isRelayNode = false;
        uint8_t emptyUUID[16] = {0};
        for (uint8_t i = 0; i < 16 ; i++) {
            if (memcmp(scannedUUID[i].UUID, emptyUUID, sizeof(emptyUUID)) != 0) {
                confirmAddDeviceTimer.start(CONFIRM_ADD_DEVICE_TIMER_MS);
                sendUartAddDevice(_uartPort, scannedUUID[i]);
                sendLogCommissionEntry(_webServer, "Start adding node " + getUUIDAsString(scannedUUID[i].UUID), "INFO");
                break;
            }
        }
    }
    else {
        if (numberOfIterations != 0 && !forceStopCommissioning) {
            qDebug() << "NUEVO ESCANEO" << numberOfIterations;
            numberOfIterations--;
            sendLogCommissionEntry(_webServer, "- New iteration in process from " + _database->getNextNodeName(doneIterations), "INFO");
            uint16_t addressToNextIt = _database->getNextNodeAddress(doneIterations);
            //qDebug() << "[1] DONE ITERATIONS: "<< doneIterations;
            delay(300);
            sendUartInyectNode(_uartPort, addressToNextIt, _database);
            while(messageState == PENDING) {}
            sendUartNewIteration(_uartPort, addressToNextIt);
            newIterationTimer.start(NEW_ITERATION_TIMER_MS);
        }
        else {
            qDebug() << "FIN DEL AUTO COMMISSION";
            forceStopCommissioning = false;
            isCommissioning = false;

            numberOfIterations = 0;
            doneIterations = 0;

            for (uint8_t i = 0; i < 20; i++) {
                memset(scannedUUID[i].UUID, 0, sizeof(scannedUUID[i].UUID));
                scannedUUID[i].nodeAddressReport = 0;
            }

            sendUartClearInyectedNodes(_uartPort, true, false, _database);
            while(messageState == PENDING) {}

            pollingTimer.start(POLLING_TIMER_MS);

            sendEndAutoCommission(_webServer);   
            cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
            embeddedState = FREE;
        }
    }
}

void Wireless::confirmAddDeviceTimerHandler()
{
    //sendUartAddDevice(_uartPort, uuidScanned);
    qDebug() << "CONFIRM ADD DEVICE";
    uint8_t emptyUUID[16] = {0};
    for (uint8_t i = 0; i < 20; i++) {
        if (memcmp(scannedUUID[i].UUID, emptyUUID, sizeof(emptyUUID)) != 0) {
            sendUartAddDevice(_uartPort, scannedUUID[i]);
            sendLogCommissionEntry(_webServer, "Start adding node " + getUUIDAsString(scannedUUID[i].UUID), "INFO");
            break;
        }
    }
}

void Wireless::newIterationTimerHandler()
{
    qDebug() << "NEW ITERATION TIMER";
    uint16_t addressToNextIt = _database->getNextNodeAddress(doneIterations);
    //qDebug() << "[4] DONE ITERATIONS: "<< doneIterations;
    delay(300);
    sendUartInyectNode(_uartPort, addressToNextIt, _database);
    while(messageState == PENDING) {}
    sendUartNewIteration(_uartPort, addressToNextIt);
}

void Wireless::replaceP2TimerHandler()
{
    deleteNodeForReplace(_webServer, _uartPort, _database);
}

void Wireless::replaceP3TimerHandler()
{
    sendLogCommissionEntry(_webServer, "The node has been deleted.", "INFO");
    restoreDataForReplace(_webServer, _uartPort, _database);
}

void Wireless::cleanCdbTimerHandler()
{
    sendUartClearInyectedNodes(_uartPort, true, true, _database);
    while(messageState == PENDING) {}
}

void Wireless::askInitDataFromMicroTimerHandler()
{
    sendSetAntennaAddressAndInstallKey(_uartPort, _database);
    while(messageState == PENDING) {}

    if(messageState == RECEIVED) {
        qDebug() << "Se han enviado los datos de inicio al micro correctamente";
    }
    else {
        qDebug() << "Fallo al intentar enviar los datos de inicio al micro";
    }
}

void Wireless::onTestButtonPressed() {
    //qDebug() << "[TEST] recibido en Wireless::onTestButtonPressed()";
    const uint16_t grupoC001 = 0xC001;
    const uint8_t  r1 = BROADCAST_ADDR;
    const uint8_t  r2 = RECALL_MAX_LVL;
    const uint8_t  ct = IS_NORMAL;

    /*qDebug() << "[TEST] TX MAX -> addr=0x" << hex << grupoC001
             << " r1=" << (int)r1 << " r2=0x" << hex << (int)r2 << " ct=" << (int)ct;*/

    if (!_uartPort) {
        qWarning() << "[TEST] _uartPort == nullptr";
        return;
    }

    sendUartDaliCommand(_uartPort, grupoC001, r1, r2, ct);

    QTimer::singleShot(10000, this, [this, grupoC001](){
        const uint8_t r1 = BROADCAST_ADDR;
        const uint8_t r2 = OFF;
        const uint8_t ct = IS_NORMAL;

        /*qDebug() << "[TEST] TX OFF -> addr=0x" << hex << grupoC001
                 << " r1=" << (int)r1 << " r2=0x" << hex << (int)r2 << " ct=" << (int)ct;*/

        if (!_uartPort) {
            qWarning() << "[TEST] _uartPort == nullptr (OFF)";
            return;
        }
        sendUartDaliCommand(_uartPort, grupoC001, r1, r2, ct);
    });

}

void Wireless::answerFactoryProgramTimerHandler()
{
    // en isFactoryProgramOn está la variable "done", que indica el éxito/fracaso del grabado
    bool done = isFactoryProgramOn;

    QString reply = QString("%1;%2;%3").arg("-", factoryProgramSerial, done ? "OK" : "FAIL"); // modelo "-" ya que no se puede leer
    QByteArray ba = reply.toUtf8();

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddressFactoryProgram);

    delay(3000);
    _udpSocket->sendData(dstAddress, ba);

    qDebug() << "SE HA GRABADO" << (isFactoryProgramOn ? "BIEN" : "MAL");

    factoryProgramSerial = "";
    isFactoryProgramOn = false;
    rcvAddressFactoryProgram = "";
}

void Wireless::identifyTimerHandler()
{
    // Parar si llegamos a tiempo máximo
    if (identifyIteration >= IDENTIFY_MAX_ITERATIONS) {
        qDebug() << "[IDENTIFY] Tiempo máximo alcanzado, deteniendo identify";
        identifyTimer.stop();
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        return;
    }

    if (identifyNodeNetAddress == 0) {
        identifyTimer.stop();
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        return;
    }

    qDebug() << "[IDENTIFY] Iteración" << identifyIteration
             << "para nodeNetAddress=0x"
             << QString::number(identifyNodeNetAddress, 16);

    sendIdentify(_uartPort, identifyNodeNetAddress);

    identifyIteration++;
}
