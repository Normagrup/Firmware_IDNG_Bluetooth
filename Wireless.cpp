#include "Wireless.h"
#include "time_functions.h"
#include "file_handler.h"
#include "process_eth_data.h"
#include "process_webserver_data.h"
#include "process_uart_data.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "log.h"
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
    connect(&testResultCheckTimer, SIGNAL(timeout()), this, SLOT(checkTestResultsHandler()));
    testResultCheckTimer.start(LOG_DATA_TIME_MS);
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
        processEthFrame(rcvAddress, dataBuffer, _udpSocket, _uartPort);
    }
}

void Wireless::uartReceivedData(QByteArray data)
{
    processUartData(data, _webServer, _uartPort, _database);
}

void Wireless::webServerReceivedData(QString data)
{
    processWebServerData(data, _webServer, _uartPort, _database);
}

void Wireless::pollingTimerHandler()
{
    if (pollingData.pollingInProgress){
        if (pollingData.pollingReceived) {
            meshDevice[subnetCount][nodeSubnetCount].setCommunicationFailure(false);
            pollingData.pollingReceived = false;
            pollingData.pollingInProgress = false;
            nodeSubnetCount++;
            goto sendNewPolling;
        }
        else if (pollingData.retries < 5) {
            pollingData.retries++;
            sendPollingFrame(_uartPort, meshDevice[subnetCount][nodeSubnetCount].getRealAddress());
            return;
        }
        else  {
            pollingData.retries = 0;
            meshDevice[subnetCount][nodeSubnetCount].setCommunicationFailure(true);
            pollingData.pollingInProgress = false;
            nodeSubnetCount++;
        }
    }

sendNewPolling:
    while (subnetCount < MAX_SUBNET) {
        while (nodeSubnetCount < MAX_NODES_SUBNET) {
            if (meshDevice[subnetCount][nodeSubnetCount].getIsConfigured()) {
                Device &device = meshDevice[subnetCount][nodeSubnetCount];

                bool lampNow = device.hasLampFailure();
                bool lampPrev = device.getPrevLampFail();
                bool batNow = device.hasBatteryFailure();
                bool batPrev = device.getPrevBatteryFail();

                bool durNow = device.hasBatteryDurationFailure();
                bool durPrev = device.getPrevDurationFail();
                bool commNow = device.hasCommunicationFailure();
                bool commPrev = device.getPrevCommFail();

                int devId = device.getRealAddress();
                QString serailNum = device.serialNumberString();
                QString devName = "SUB:" + QString::number(subnetCount) + " " + "ID:" + QString::number(nodeSubnetCount);
                AntennaInfo info = getAntennaInfo(_database);
                QString eventType = "Fail";

                if (lampNow != lampPrev) {
                    if (lampNow) {
                        insertLogEvent(_database, devId, serailNum, devName, info.ip, info.timestamp, LOG_LAMP_FAILURE, eventType);
                    } else {
                        insertLogEvent(_database, devId, serailNum, devName, info.ip, info.timestamp, LOG_LAMP_RECOVERED, eventType);
                    }
                    device.setPrevLampFail(lampNow);
                }
                if (batNow != batPrev) {
                    if (batNow) {
                        insertLogEvent(_database, devId, serailNum, devName, info.ip, info.timestamp, LOG_BATTERY_FAILURE, eventType);
                    } else {
                        insertLogEvent(_database, devId, serailNum, devName,info.ip, info.timestamp, LOG_BATTERY_RECOVERED, eventType);
                    }
                    device.setPrevBatteryFail(batNow);
                }
                if (durNow != durPrev) {
                    if (durNow) {
                        insertLogEvent(_database, devId, serailNum, devName, info.ip, info.timestamp, LOG_DURATION_FAILURE, eventType);
                    } else {
                        insertLogEvent(_database, devId, serailNum, devName, info.ip, info.timestamp, LOG_DURATION_RECOVERED, eventType);
                    }
                    device.setPrevDurationFail(durNow);
                }
                if (commNow != commPrev) {
                    if (commNow) {
                        insertLogEvent(_database, devId, serailNum, devName, info.ip, info.timestamp, LOG_COMMUNICATION_FAILURE, eventType);
                    } else {
                        insertLogEvent(_database, devId, serailNum, devName, info.ip, info.timestamp, LOG_COMMUNICATION_RECOVERED, eventType);
                    }
                    device.setPrevCommFail(commNow);
                }

                sendPollingFrame(_uartPort, meshDevice[subnetCount][nodeSubnetCount].getRealAddress());
                pollingData.pollingInProgress = true;
                return;
            }
            nodeSubnetCount++;
        }
        nodeSubnetCount = 0;
        subnetCount++;
    }

    nodeSubnetCount = 0;
    subnetCount = 0;
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

                    int devId = tests[i].getGroupAddress().toUInt(NULL, 16);
                    QString serailNum = "FF.FF.FF.FF";
                    QString devName = "Group: " + tests[i].getGroupAddress();
                    QString eventType = "Test";
                    AntennaInfo info = getAntennaInfo(_database);

                    insertLogEvent(_database, devId, serailNum, devName, info.ip, info.timestamp, LOG_TEST_REQUESTED_FUNCTIONAL, eventType);

                    qDebug() << "FUNCTIONAL TEST";
                    AntennaTestCheck testCheck;
                    testCheck.groupId = devId;
                    testCheck.testType = "FUNCTIONAL";
                    testCheck.checkTime = info.timestamp.time().addSecs(900); // 15 min for FT;

                    antennaTestCheckList.append(testCheck);
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
                int devId = tests[i].getGroupAddress().toUInt(NULL, 16);

                QString serailNum = "FF.FF.FF.FF";
                QString devName = "Group: " + tests[i].getGroupAddress();
                QString eventType = "Test";
                AntennaInfo info = getAntennaInfo(_database);

                insertLogEvent(_database, devId, serailNum, devName, info.ip, info.timestamp, LOG_TEST_REQUESTED_DURATION, eventType);

                qDebug() << "DURATION TEST";
                AntennaTestCheck testCheck;
                testCheck.groupId = devId;
                testCheck.testType = "DURATION";
                testCheck.checkTime = info.timestamp.time().addSecs(43200); // 12 hour for DT

                antennaTestCheckList.append(testCheck);
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

                        int devId = tests[i].getGroupAddress().toUInt(NULL, 16);
                        QString serailNum = "FF.FF.FF.FF";
                        QString devName = "Group: " + tests[i].getGroupAddress();
                        QString eventType = "Test";
                        AntennaInfo info = getAntennaInfo(_database);

                        insertLogEvent(_database, devId, serailNum, devName, info.ip, info.timestamp, LOG_TEST_REQUESTED_DURATION, eventType);

                        qDebug() << "DURATION TEST";
                        AntennaTestCheck testCheck;
                        testCheck.groupId = devId;
                        testCheck.testType = "DURATION";
                        testCheck.checkTime = info.timestamp.time().addSecs(43200); // 12 hour for DT

                        antennaTestCheckList.append(testCheck);
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
                    int devId = realAddress;
                    QString serial = device.serialNumberString();
                    QString name = "SUB:" + QString::number(subnet) + " ID:" + QString::number(node);

                    if (check.testType == "FUNCTIONAL") {
                        insertLogEvent(_database, devId, serial, name, info.ip, info.timestamp, LOG_TEST_COMPLETED_FUNCTIONAL, eventType);
                        insertLogEvent(_database, devId, serial, name, info.ip, info.timestamp,
                                       failed ? LOG_TEST_FT_FAIL : LOG_TEST_FT_OK, eventType);
                    } else {
                        insertLogEvent(_database, devId, serial, name, info.ip, info.timestamp, LOG_TEST_COMPLETED_DURATION, eventType);
                        insertLogEvent(_database, devId, serial, name, info.ip, info.timestamp,
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

    if (commissionData.isRelayNode && scannedUUID[0].nodeAddressReport != 0x0001) {
        do {
            qDebug() << "CHANGE RELAY NODE" << scannedUUID[0].nodeAddressReport;
            sendUartChangeRelay(_uartPort, scannedUUID[0].nodeAddressReport);
            delay(5000);
        } while (!commissionData.isChangeRelayConfirmed);
    }

    qDebug() << "IS CHANGE FALSE";
    commissionData.isChangeRelayConfirmed = false;

    if (commissionData.numberOfNodesScanned > 0 && !forceStopCommissioning) {
        qDebug() << "EMPEZAMOS A AÑADIR NODOS";
        sendLogCommissionEntry(_webServer, "Start adding nodes...");
        commissionData.isRelayNode = false;
        uint8_t emptyUUID[16] = {0};
        for (uint8_t i = 0; i < 16 ; i++) {
            if (memcmp(scannedUUID[i].UUID, emptyUUID, sizeof(emptyUUID)) != 0) {
                confirmAddDeviceTimer.start(CONFIRM_ADD_DEVICE_TIMER_MS);
                sendUartAddDevice(_uartPort, scannedUUID[i]);
                sendLogCommissionEntry(_webServer, "Start adding node " + getUUIDAsString(scannedUUID[i].UUID));
                break;
            }
        }
    }
    else {
        if (numberOfIterations != 0 && !forceStopCommissioning) {
            qDebug() << "NUEVO ESCANEO" << numberOfIterations;
            sendLogCommissionEntry(_webServer, "Starting new iteration from a node...");
            sendUartNewIteration(_uartPort);
            newIterationTimer.start(NEW_ITERATION_TIMER_MS);
        }
        else {
            qDebug() << "FIN DEL AUTO COMMISSION";
            forceStopCommissioning = false;
            isCommissioning = false;

            numberOfIterations = 0;

            for (uint8_t i = 0; i < 20; i++) {
                memset(scannedUUID[i].UUID, 0, sizeof(scannedUUID[i].UUID));
                scannedUUID[i].nodeAddressReport = 0;
            }

            sendEndAutoCommission(_webServer);
            pollingTimer.start(POLLING_TIMER_MS);
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
            sendLogCommissionEntry(_webServer, "Start adding node " + getUUIDAsString(scannedUUID[i].UUID));
            break;
        }
    }
}

void Wireless::newIterationTimerHandler()
{
    qDebug() << "NEW ITERATION TIMER";
    sendUartNewIteration(_uartPort);
}


