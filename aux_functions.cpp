#include "aux_functions.h"
#include "file_handler.h"
#include "log.h"

#include <QDebug>

uint16_t getTargetAddress(uint8_t subnet, uint8_t daliAddress)
{
    if (subnet == SUBNET_GROUPS) { return (0xC100 + daliAddress); }
    else { return meshDevice[subnet][daliAddress].getRealAddress(); }
}

void QStringToUint8Array(const QString &hexString, uint8_t array[16])
{
    QByteArray byteArray = hexString.toLatin1();
    const char *data = byteArray.constData();

    for (int i = 15; i >= 0; i--) {
        char hex[3] = { data[(15 - i) * 2], data[(15 - i) * 2 + 1], '\0' };
        array[i] = strtoul(hex, nullptr, 16);
    }
}

ScannedUUID compareDeviceUUID(QString receivedUUID)
{
    uint8_t uuid[16];
    QStringToUint8Array(receivedUUID, uuid);
    ScannedUUID returnVar;
    //returnVar.UUID = nullptr;

    for (uint8_t i = 0; i < (MAX_SUBNET * MAX_NODES_SUBNET); i++) {
        if (!memcmp(scannedUUID[i].UUID, uuid, sizeof(uuid))) {
            return scannedUUID[i];
        }
    }

    return returnVar;
}

uint16_t getNodeNetAddress(QString receivedData)
{
    receivedData.remove("Node");
    receivedData = receivedData.trimmed();
    return receivedData.toUInt();
}

uint16_t* getGroupAddress(QString receivedData)
{
    receivedData.remove("Node");
    receivedData = receivedData.trimmed();

    QStringList parts = receivedData.split(" ");
    if (parts.size() == 2) {
        uint16_t nodeNetAddress = parts[0].toUShort();
        uint16_t groupAddress = parts[1].toInt(nullptr, 16);

        uint16_t* address = new uint16_t[2];
        address[0] = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress();
        address[1] = groupAddress;

        return address;
    }
}

uint16_t getOneGroupAddress(QString receivedData)
{
    receivedData = receivedData.trimmed();

    uint16_t groupAddress = receivedData.toInt(nullptr, 16);

    return groupAddress;
}

uint16_t* getActualLvl(QString receivedData)
{
    QStringList parts = receivedData.split(" ");
    if (parts.size() == 2) {
        uint16_t nodeNetAddress = parts[0].toUShort();
        uint16_t actualLvl = parts[1].toInt(nullptr, 10);
        actualLvl = actualLvl * 254 / 100;

        uint16_t* values = new uint16_t[2];
        if (nodeNetAddress < 0xC000) { values[0] = meshDevice[(nodeNetAddress - 1) / 64][(nodeNetAddress - 1) % 64].getRealAddress(); }
        else { values[0] = nodeNetAddress; }
        values[1] = actualLvl;

        return values;
    }
}

void setFirstAddressAvailable(uint16_t nodeAddress, uint8_t* nodeUUID, Database* database, uint8_t* netAddress)
{
    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (!meshDevice[i][j].getIsConfigured()) {
                //meshDevice[i][j].setIsConfigured(true);
                meshDevice[i][j].setRealAddress(nodeAddress);
                meshDevice[i][j].setUUID(nodeUUID);
                database->setNewNode(i, j, nodeAddress, nodeUUID, 12345);
                netAddress[0] = i;
                netAddress[1] = j;
                return;
            }
        }
    }
}

void convertUuidStringToByteArray(QString uuidString, uint8_t* UUID)
{
    for (uint8_t i = 0; i < 16; i++) {
        QString byteString = uuidString.mid(i * 2, 2);
        bool ok;
        uint8_t byte = byteString.toUInt(&ok, 16);
        if (ok) { UUID[i] = byte; }
    }
}

uint8_t convertGroupSubStringToArray(QString groupSubString, uint16_t* groupSubArray)
{
    QStringList groupList = groupSubString.split(", ");
    uint8_t groupCount = 0;

    for (const QString& group : groupList) {
        bool ok;
        uint16_t groupAddress = group.toUShort(&ok, 16);
        if (ok && groupCount < MESH_GROUP_COUNT) { groupSubArray[groupCount++] = groupAddress;  }
    }

    return groupCount;
}

void setIPConfigInfo(QStringList webServerParts, Database* database)
{
    QString ip, submask, gateway;

    ip = webServerParts[0];
    submask = webServerParts[1];
    gateway = webServerParts[2];

    setIPAddressFile(ip);
    setSubmaskAddressFile(submask);
    setGatewayAddressFile(gateway);

    database->setInterfaceParameters(webServerParts);
}

int getUUIDIndexOfScanned(QString UUID)
{
    // Para pasar de "FFFFFFFFF49A1EA75C47AB0C0B65E158" a [255, 255, 255, 255, 244, 154, 30, ...]
    // Es decir, parte el UUID en paquetes de 2 caracteres, lo convierte a hexadecimal y lo mete en "parts"
    uint8_t parts[16];
    for(int i = 0; i < 16; i++)
        parts[i] = static_cast<uint8_t>(UUID.mid(i*2, 2).toUInt(nullptr, 16));

    for (int i = 0; i < 20; i++) {
        ScannedUUID uuid = scannedUUID[i];
        for (int j = 0; j < 16; j++) {
            if(uuid.UUID[j] != parts[j])
                break;
            else if(uuid.UUID[j] == parts[j] && j == 15)
                return i;
        }
    }

    return -1;

    /** PARA PRINTEAR EL CONTENIDO DE SCANNEDUUID
    for(int i = 0; i < 20; i++) {
        qDebug() << "+++ UUID" << i << ":" << scannedUUID[i].nodeAddressReport;
        for(int j = 0; j < 16; j++) {
            qDebug() << "Pos" << j << ":" << scannedUUID[i].UUID[j];
        }
    }
    */
}

QString getUUIDAsString(uint8_t UUID[16])
{
    QByteArray byteArray(reinterpret_cast<const char*>(UUID), 16);
    return byteArray.toHex().toUpper();
}

void setTests(QStringList webServerParts, Database *database)
{
    QString groupAddress = webServerParts[0];
    QString functionalEnable = webServerParts[1];
    QString functionalDaysWebserver = webServerParts[2];
    QString functionalTime = webServerParts[3];
    QString durationEnable = webServerParts[4];
    QString durationPeriodicity = webServerParts[5];
    QString durationDate = webServerParts[6];
    QString durationTime = webServerParts[7];

    QStringList functionalDaysWebserverParts = functionalDaysWebserver.split("-");
    QString functionalDays;
    for (uint8_t i = 0; i < functionalDaysWebserverParts.size(); i++) { functionalDays += functionalDaysWebserverParts[i] + " "; }

    for (uint8_t i = 0; i < MAX_TEST; i++) {
        if (tests[i].getGroupAddress() == groupAddress) {
            tests[i].setFunctionalEnable(functionalEnable == "1");
            tests[i].setDurationEnable(durationEnable == "1");
            tests[i].setFunctionalDays(functionalDays);
            tests[i].setFunctionalTime(functionalTime);
            tests[i].setDurationPeriodicity(durationPeriodicity);
            tests[i].setDurationDate(durationDate);
            tests[i].setDurationTime(durationTime);

            database->setTestEnable(tests[i].getGroupAddress(), functionalEnable == "1", durationEnable == "1");
            database->setFunctionalTest(tests[i].getGroupAddress(), functionalDays, functionalTime);
            database->setDurationTest(tests[i].getGroupAddress(), durationPeriodicity, durationDate, durationTime);
        }
    }
}

void transformEventCodes(QList<QStringList>* logs)
{
    for (QStringList& entry : *logs) {
        QString& event = entry[5];  // Campo Event

        if (event == "209") { event = "DEVICE ADDED"; }
        else if (event == "210") { event = "DEVICE ERROR"; }
        else if (event == "211") { event = "GROUP FAIL"; }
        else if (event == "212") { event = "DEV TYPE FAIL"; }
        else if (event == "213") { event = "NET ADDR FAIL"; }
        else if (event == "214") { event = "DEVICE REMOVED"; }

        else if (event == "1") { event = "COMMUNICATION FAIL"; }
        else if (event == "16") { event = "COMMUNICATION RECOVERY"; }

        else if (event == "2") { event = "BATTERY FAIL"; }
        else if (event == "32") { event = "BATTERY RECOVERY"; }

        else if (event == "3") { event = "LAMP FAIL"; }
        else if (event == "48") { event = "LAMP RECOVERY"; }

        else if (event == "4") { event = "DURATION FAIL"; }
        else if (event == "64") { event = "DURATION RECOVERY"; }

        else if (event == "8") { event = "FUN TEST REQUESTED"; }
        else if (event == "9") { event = "DUR TEST REQUESTED"; }
        else if (event == "10") { event = "TEST STOPPED"; }

        else if (event == "11") { event = "FUN TEST COMPLETED"; }
        else if (event == "12") { event = "DUR TEST COMPLETED"; }

        else if (event == "177") { event = "FUN TEST OK"; }
        else if (event == "178") { event = "FUN TEST FAIL"; }

        else if (event == "193") { event = "DUR TEST OK"; }
        else if (event == "194") { event = "DUR TEST FAIL"; }

        else if (event == "80") { event = "ADD TO GROUP OK"; }
        else if (event == "81") { event = "ADD TO GROUP FAIL"; }
        else if (event == "87") { event = "DEL FROM GROUP"; }

        else if (event == "82") { event = "GROUP CREATED"; }
        else if (event == "83") { event = "GROUP DELETED"; }

        else if (event == "84") { event = "POL TO MAX"; }
        else if (event == "85") { event = "POL TO LV"; }
        else if (event == "86") { event = "POL TO OFF"; }
    }
}

void insertLogEvent(Database *database, QString name, QString serialNum, int btAddress, QString devIP, QDateTime dateTime, int eventCode, QString eventType)
{
    LogInfo log;
    log.name = name;
    log.serialNum = serialNum;
    log.btAddress = btAddress;
    log.devIP = devIP;
    log.timestamp = dateTime.toSecsSinceEpoch();
    log.event = eventCode;
    log.eventType = eventType;

    database->insertLogEvent(log);
}

void logTestRequest(Database* db, uint16_t targetAddr, bool isGroup, const QString& testType)
{
    QString serial = isGroup ? "FF.FF.FF.FF"
                             : meshDevice[(targetAddr - 1) / 64][(targetAddr - 1) % 64].serialNumberString();

    QString name;
    if(isGroup) {
        QString hexAddr = QString("%1").arg(targetAddr, 4, 16, QChar('0')).toUpper();
        name = db->getGroupName(hexAddr) + " [G]";
    } else {
        name = "SUB:" + QString::number((targetAddr - 1) / 64) + " ID:" + QString::number((targetAddr - 1) % 64);
    }

    int btAddress = isGroup
                    ? targetAddr
                    : meshDevice[(targetAddr - 1) / 64][(targetAddr - 1) % 64].getRealAddress();

    AntennaInfo info = getAntennaInfo(db);
    QString eventType = "Test";

    int logType = 0;
    if (testType == "FUNCTIONAL") { logType = LOG_TEST_REQUESTED_FUNCTIONAL; }
    else if (testType == "DURATION") { logType = LOG_TEST_REQUESTED_DURATION; }
    else if (testType == "STOP") { logType = LOG_TEST_STOPPED; }

    insertLogEvent(db, name, serial, btAddress, info.ip, info.timestamp, logType, eventType);

    if (testType == "FUNCTIONAL" || testType == "DURATION") {
        addTestToChecklist(btAddress, testType, info.timestamp);
    }
    if (testType == "STOP"){
        removeLogTestFromCheckList(btAddress);
    }
}

void addTestToChecklist(uint16_t realAddr, const QString& testType, const QDateTime& baseTime)
{
    int delay = (testType == "FUNCTIONAL") ? 900 : 43200; // FT 15 min, DT 12 hours
    QTime checkTime = baseTime.time().addSecs(delay);

    AntennaTestCheck check;
    check.groupId = realAddr;
    check.testType = testType;
    check.checkTime = checkTime;

    antennaTestCheckList.append(check);
}

void insertDevToLog(uint16_t nodeAddress, Database *db, int eventCode, QString eventType)
{
    QString name, serial;
    int btAddress;

    for (uint8_t i = 0; i < MAX_SUBNET; i++) {
        for (uint8_t j = 0; j < MAX_NODES_SUBNET; j++) {
            if (meshDevice[i][j].getRealAddress() == nodeAddress) {
                name = "SUB:" + QString::number(i) + " ID:" + QString::number(j);
                serial = meshDevice[i][j].serialNumberString();
                btAddress = nodeAddress;
            }
        }
    }

    AntennaInfo info = getAntennaInfo(db);

    insertLogEvent(db, name, serial, btAddress, info.ip, info.timestamp, eventCode, eventType);
}

AntennaInfo getAntennaInfo(Database *db)
{
    QString date = getLocalDate();
    QString time = getLocalTime();
    QDateTime timestamp = QDateTime::fromString(date + " " + time, "yyyy-MM-dd HH:mm:ss");
    QString netIp = db->getInterfaceParameters().first();

    return { timestamp, netIp };
}

void removeLogTestFromCheckList(uint16_t nodeAddress)
{
    for (int i = 0; i < antennaTestCheckList.size(); ) {
        if (antennaTestCheckList[i].groupId == nodeAddress) {
            antennaTestCheckList.removeAt(i);
        } else {
            ++i;
        }
    }
}

void insertCommissionErrorToLog(const QByteArray& uuidArray, Database *db, int eventCode)
{
    if (uuidArray.size() < 16) return;

    QString name = QString("DEV ERR: %1").arg(currentNodeAddress);
    int btAddress = currentNodeAddress;
    QString serial = QString("%1.%2.%3.%4")
                         .arg(static_cast<uint8_t>(uuidArray[15]), 2, 16, QChar('0'))
                         .arg(static_cast<uint8_t>(uuidArray[14]), 2, 16, QChar('0'))
                         .arg(static_cast<uint8_t>(uuidArray[13]), 2, 16, QChar('0'))
                         .arg(static_cast<uint8_t>(uuidArray[12]), 2, 16, QChar('0'))
                         .toUpper();

    AntennaInfo info = getAntennaInfo(db);
    QString eventType = "Commissioning";

    insertLogEvent(db, name, serial, btAddress, info.ip, info.timestamp, eventCode, eventType);
    currentNodeAddress = 0;
}
