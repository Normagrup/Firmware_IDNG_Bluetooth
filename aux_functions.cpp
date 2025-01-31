#include "aux_functions.h"
#include "file_handler.h"

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
                meshDevice[i][j].setSerialNumber(nodeUUID);
                database->setNewNode(i, j, nodeAddress, nodeUUID);
                netAddress[0] = i;
                netAddress[1] = j;
                return;
            }
        }
    }
}

void convertUuidStringToByteArray(QString uuid, uint8_t* serialNumber)
{
    for (uint8_t i = 0; i < 16; i++) {
        QString byteString = uuid.mid(i * 2, 2);
        bool ok;
        uint8_t byte = byteString.toUInt(&ok, 16);
        if (ok) { serialNumber[15 - i] = byte; }
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

void disableAllTest(QStringList webServerParts, Database* database)
{
    QString groupAddress = webServerParts[0];

    for (uint8_t i = 0; i < MAX_TEST; i++) {
        if (tests[i].getGroupAddress() == groupAddress) {
            tests[i].setFunctionalEnable(false);
            tests[i].setDurationEnable(false);

            database->setTestEnable(tests[i].getGroupAddress(), false, false);
        }
    }
}

void setFunctionalTest(QStringList webServerParts, Database *database)
{
    QString groupAddress = webServerParts[0];
    QString functionalTime = webServerParts[2];
    QString functionalDaysWebserver = webServerParts[1];
    QStringList functionalDaysWebserverParts = functionalDaysWebserver.split("-");
    QString functionalDays;
    for (uint8_t i = 0; i < functionalDaysWebserverParts.size(); i++) { functionalDays += functionalDaysWebserverParts[i] + " "; }

    for (uint8_t i = 0; i < MAX_TEST; i++) {
        if (tests[i].getGroupAddress() == groupAddress) {
            tests[i].setFunctionalEnable(true);
            tests[i].setDurationEnable(false);
            tests[i].setFunctionalDays(functionalDays);
            tests[i].setFunctionalTime(functionalTime);

            database->setTestEnable(tests[i].getGroupAddress(), true, false);
            database->setFunctionalTest(tests[i].getGroupAddress(), functionalDays, functionalTime);
        }
    }
}

void setDurationTest(QStringList webServerParts, Database *database)
{
    QString groupAddress = webServerParts[0];
    QString durationPeriodicity = webServerParts[1];
    QString durationDate = webServerParts[2];
    QString durationTime = webServerParts[3];

    for (uint8_t i = 0; i < MAX_TEST; i++) {
        if (tests[i].getGroupAddress() == groupAddress) {
            tests[i].setFunctionalEnable(false);
            tests[i].setDurationEnable(true);
            tests[i].setDurationPeriodicity(durationPeriodicity);
            tests[i].setDurationDate(durationDate);
            tests[i].setDurationTime(durationTime);

            database->setTestEnable(tests[i].getGroupAddress(), false, true);
            database->setDurationTest(tests[i].getGroupAddress(), durationPeriodicity, durationDate, durationTime);
        }
    }
}

void setAllTest(QStringList webServerParts, Database *database)
{
    QString groupAddress = webServerParts[0];
    QString functionalTime = webServerParts[2];
    QString durationPeriodicity = webServerParts[3];
    QString durationDate = webServerParts[4];
    QString durationTime = webServerParts[5];
    QString functionalDaysWebserver = webServerParts[1];
    QStringList functionalDaysWebserverParts = functionalDaysWebserver.split("-");
    QString functionalDays;
    for (uint8_t i = 0; i < functionalDaysWebserverParts.size(); i++) { functionalDays += functionalDaysWebserverParts[i] + " "; }

    for (uint8_t i = 0; i < MAX_TEST; i++) {
        if (tests[i].getGroupAddress() == groupAddress) {
            tests[i].setFunctionalEnable(true);
            tests[i].setDurationEnable(true);
            tests[i].setFunctionalDays(functionalDays);
            tests[i].setFunctionalTime(functionalTime);
            tests[i].setDurationPeriodicity(durationPeriodicity);
            tests[i].setDurationDate(durationDate);
            tests[i].setDurationTime(durationTime);

            database->setTestEnable(tests[i].getGroupAddress(), true, true);
            database->setFunctionalTest(tests[i].getGroupAddress(), functionalDays, functionalTime);
            database->setDurationTest(tests[i].getGroupAddress(), durationPeriodicity, durationDate, durationTime);
        }
    }
}
