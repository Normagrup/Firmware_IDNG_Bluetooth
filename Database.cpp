#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "Database.h"
#include "global_variables.h"
#include "aux_functions.h"
#include "time_functions.h"
#include "file_handler.h"

Database::Database(QObject *parent)
    : QObject{parent}
{
    _database = QSqlDatabase::addDatabase("QSQLITE");
    _database.setDatabaseName("/projects/IDNG_Blue/sql/network");

    initDatabase();
}

void Database::initDatabase()
{
    openDatabase();

    QSqlQuery query;

    /* **************************************************
     *                                                  *
     *                      NODES                       *
     *                                                  *
     * **************************************************/
    query.exec("CREATE TABLE IF NOT EXISTS Nodes "
               "(SubnetAddress	INTEGER, "
               "NodeSubnetAddress INTEGER, "
               "RealAddress INTEGER, "
               "UUID TEXT, "
               "GroupSub TEXT, "
               "DeviceType INTEGER, "
               "RatedDuration INTEGER, "
               "EmergencyFeatures INTEGER, "
               "PhysicalMinLvl INTEGER);");



    /* **************************************************
     *                                                  *
     *                      USERS                       *
     *                                                  *
     * **************************************************/
    query.exec("CREATE TABLE IF NOT EXISTS Users "
               "(Name TEXT, "
               "Password TEXT, "
               "ID INTEGER);");


    query.prepare("SELECT COUNT(*) FROM Users WHERE Name = :name");
    query.bindValue(":name", "admin");

    if (!query.exec()) { qDebug() << "Error executing SELECT query in Users:" << query.lastError().text(); }
    else {
        if (query.next() && query.value(0).toInt() == 0) {
            query.prepare("INSERT INTO Users (Name, Password, ID) VALUES (:name1, :password1, :id1), (:name2, :password2, :id2)");

            query.bindValue(":name1", "admin");
            query.bindValue(":password1", "A1234");
            query.bindValue(":id1", 1);

            query.bindValue(":name2", "fabrica");
            query.bindValue(":password2", "fabrica");
            query.bindValue(":id2", 2);

            if (!query.exec()) { qDebug() << "Error executing INSERT query in Users:" << query.lastError().text(); }
        }
    }



    /* **************************************************
     *                                                  *
     *                    GENERAL                       *
     *                                                  *
     * **************************************************/
    query.exec("CREATE TABLE IF NOT EXISTS General "
               "(IP TEXT, "
               "Submask TEXT, "
               "Gateway TEXT, "
               "BuildingName TEXT, "
               "LineName TEXT);");

    query.prepare("SELECT * FROM General");

    QString ip, submask, gateway;
    ip = getInterfacesConfig("address");
    submask = getInterfacesConfig("netmask");
    gateway = getInterfacesConfig("gateway");

    if (!query.exec()) { qDebug() << "Error executing SELECT query in Users:" << query.lastError().text(); }
    else {
        if (!query.next()) {
            query.prepare("INSERT INTO General (IP, Submask, Gateway, BuildingName, LineName) VALUES (:ip, :submask, :gateway, :buildingName, :lineName)");
            query.bindValue(":ip", ip);
            query.bindValue(":submask", submask);
            query.bindValue(":gateway", gateway);
            query.bindValue(":buildingName", "NO_NAME");
            query.bindValue(":lineName", "NO_NAME");

            if (!query.exec()) { qDebug() << "Error executing INSERT query in Users:" << query.lastError().text(); }
        }
    }



    /* **************************************************
     *                                                  *
     *                      TEST                        *
     *                                                  *
     * **************************************************/
    query.exec("CREATE TABLE IF NOT EXISTS Test "
               "(GroupAddress TEXT, "
               "FunctionalEnable INTEGER, "
               "DurationEnable INTEGER, "
               "FunctionalDays TEXT, "
               "FunctionalTime TEXT, "
               "DurationPeriodicity TEXT, "
               "DurationDate TEXT, "
               "DurationTime TEXT);");

    query.prepare("SELECT * FROM Test");

    if (!query.exec()) { qDebug() << "Error executing SELECT query in Test:" << query.lastError().text(); }
    else {
        if (!query.next()) {
            QStringList groupAddresses = {"C010", "C011", "C012", "C013", "C014", "C015", "C016", "C017", "C018", "C019", "C01A", "C01B", "C01C", "C01D", "C01E", "C01F", "FFFF"};

            query.prepare("INSERT INTO Test (GroupAddress, FunctionalEnable, DurationEnable, FunctionalDays, FunctionalTime, DurationPeriodicity, DurationDate, DurationTime) "
                          "VALUES (:groupAddress, :functionalEnable, :durationEnable, :functionalDays, :functionalTime, :durationPeriodicity, :durationDate, :durationTime)");

            foreach (const QString &groupAddress, groupAddresses) {
                query.bindValue(":groupAddress", groupAddress);
                query.bindValue(":functionalEnable", 0);
                query.bindValue(":durationEnable", 0);
                query.bindValue(":functionalDays", " ");
                query.bindValue(":functionalTime", "00:00");
                query.bindValue(":durationPeriodicity", "0");
                query.bindValue(":durationDate", "0000-00-00");
                query.bindValue(":durationTime", "00:00");

                if (!query.exec()) { qDebug() << "Error executing INSERT query in Test:" << query.lastError().text(); }
            }
        }
    }
}

bool Database::openDatabase()
{
    return _database.open();
}

void Database::closeDatabase()
{
    _database.close();
}

uint8_t Database::verifyLoginParameters(QString loginParameters)
{
    QStringList loginCredentials = loginParameters.split(" ");
    QString username = loginCredentials[0];
    QString password = loginCredentials[1];

    QStringList databaseLoginParametersList = this->getLoginParameters();

    for (const QString& databaseEntry : databaseLoginParametersList) {
        QStringList databaseParts = databaseEntry.split(" ");

        if (username == databaseParts[0] && password == databaseParts[1] && username == "admin") { return 1; }
        else if (username == databaseParts[0] && password == databaseParts[1] && username == "fabrica") { return 2; }
    }

    return 0;
}

QStringList Database::getLoginParameters()
{
    QSqlQuery query;

    query.prepare("SELECT Name, Password FROM Users");

    if (!query.exec()) { return {}; }

    QStringList loginParametersList;
    while (query.next()) {
        QString username = query.value("Name").toString();
        QString password = query.value("Password").toString();
        loginParametersList.append(username + " " + password);
    }

    return loginParametersList;
}

QString Database::getInterfaceParameters()
{
    QSqlQuery query;

    query.prepare("SELECT * FROM General");

    if (!query.exec()) { return ""; }

    QString ip, submask, gateway, buildingName, lineName;
    if (query.next()) {
        ip = query.value("IP").toString();
        submask = query.value("Submask").toString();
        gateway = query.value("Gateway").toString();
        buildingName = query.value("BuildingName").toString();
        lineName = query.value("LineName").toString();
    }

    return (ip + " " + submask + " " + gateway + " " + buildingName + " " + lineName);
}

void Database::setInterfaceParameters(QStringList interfaceParameters)
{
    QString ip, submask, gateway, buildingName, lineName;

    ip = interfaceParameters[0];
    submask = interfaceParameters[1];
    gateway = interfaceParameters[2];
    buildingName = interfaceParameters[3];
    lineName = interfaceParameters[4];

    QSqlQuery query;
    query.prepare("UPDATE General SET IP = :ip, Submask = :submask, Gateway = :gateway, BuildingName = :buildingName, LineName = :lineName");
    query.bindValue(":ip", ip);
    query.bindValue(":submask", submask);
    query.bindValue(":gateway", gateway);
    query.bindValue(":buildingName", buildingName);
    query.bindValue(":lineName", lineName);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in setInterfaceParameters" << query.lastError().text(); }
}

void Database::loadNodesFromDatabase()
{
    QSqlQuery query;
    if (!query.exec("SELECT * FROM Nodes")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    while (query.next()) {
        uint8_t subnetAddress = query.value("SubnetAddress").toUInt();
        uint8_t nodeSubnetAddress = query.value("NodeSubnetAddress").toUInt();
        QString uuid = query.value("UUID").toString();
        QString groupSub = query.value("GroupSub").toString();

        uint8_t serialNumber[16];
        convertUuidStringToByteArray(uuid, serialNumber);
        meshDevice[subnetAddress][nodeSubnetAddress].setSerialNumber(serialNumber);

        uint16_t groupSubAddresses[MESH_GROUP_COUNT];
        uint8_t groupCount = convertGroupSubStringToArray(groupSub, groupSubAddresses);
        for (uint8_t i = 0; i < groupCount; i++) { meshDevice[subnetAddress][nodeSubnetAddress].setGroupSubAddress(groupSubAddresses[i]); }

        meshDevice[subnetAddress][nodeSubnetAddress].setIsConfigured(true);
        meshDevice[subnetAddress][nodeSubnetAddress].setRealAddress(query.value("RealAddress").toUInt());
        meshDevice[subnetAddress][nodeSubnetAddress].setDeviceType(query.value("DeviceType").toUInt());
        meshDevice[subnetAddress][nodeSubnetAddress].setRatedDuration(query.value("RatedDuration").toUInt());
        meshDevice[subnetAddress][nodeSubnetAddress].setEmergencyFeatures(query.value("EmergencyFeatures").toUInt());
        meshDevice[subnetAddress][nodeSubnetAddress].setPhysicalMinLvl(query.value("PhysicalMinLvl").toUInt());
    }
}

void Database::loadTestsFromDatabase()
{
    QSqlQuery query;
    if (!query.exec("SELECT * FROM Test")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    uint8_t testCounter = 0;

    while (query.next() && testCounter < MAX_TEST) {
        QString groupAddress = query.value("GroupAddress").toString();
        bool isFunctionalEnable = query.value("FunctionalEnable").toUInt();
        bool isDurationEnable = query.value("DurationEnable").toUInt();
        QString functionalDays = query.value("FunctionalDays").toString();
        QString functionalTime = query.value("FunctionalTime").toString();
        QString durationPeriodicity = query.value("DurationPeriodicity").toString();
        QString durationDate = query.value("DurationDate").toString();
        QString durationTime = query.value("DurationTime").toString();

        tests[testCounter].setGroupAddress(groupAddress);
        tests[testCounter].setFunctionalEnable(isFunctionalEnable);
        tests[testCounter].setDurationEnable(isDurationEnable);
        tests[testCounter].setFunctionalDays(functionalDays);
        tests[testCounter].setFunctionalTime(functionalTime);
        tests[testCounter].setDurationPeriodicity(durationPeriodicity);
        tests[testCounter].setDurationDate(durationDate);
        tests[testCounter].setDurationTime(durationTime);

        testCounter++;
    }
}

void Database::setNewNode(uint8_t subnetAddress, uint8_t nodeSubnetAddress, uint16_t realAddress, uint8_t *nodeUUID)
{
    QString nodeUUIDText;
    for (int8_t i = 15; i >= 0; i--) { nodeUUIDText += QString::asprintf("%02X", nodeUUID[i]); }

    QSqlQuery query;

    query.prepare("SELECT COUNT(*) FROM Nodes WHERE UUID = :uuid");
    query.bindValue(":uuid", nodeUUIDText);

    if (!query.exec()) {
        qDebug() << "Error executing SELECT query in setNewNode:" << query.lastError().text();
        return;
    }

    int count = 0;
    if (query.next()) { count = query.value(0).toInt(); }

    if (count == 0) {
        query.prepare("INSERT INTO Nodes (SubnetAddress, NodeSubnetAddress, RealAddress, UUID) VALUES (:subnetAddress, :nodeSubnetAddress, :realAddress, :uuid)");
        query.bindValue(":subnetAddress", subnetAddress);
        query.bindValue(":nodeSubnetAddress", nodeSubnetAddress);
        query.bindValue(":realAddress", realAddress);
        query.bindValue(":uuid", nodeUUIDText);

        if (!query.exec()) { qDebug() << "Error executing INSERT query in setNewNode:" << query.lastError().text(); }
    }
}


void Database::setGroup(uint16_t realAddress, uint16_t groupAddress)
{
    QSqlQuery query;

    query.prepare("SELECT GroupSub FROM Nodes WHERE RealAddress = :realAddress");
    query.bindValue(":realAddress", realAddress);

    if (!query.exec()) { return; }

    QString groupCell;
    if (query.next()) { groupCell = query.value(0).toString(); }

    QString newCell;
    if (groupCell.isEmpty()) { newCell = QString::number(groupAddress, 16).toUpper(); }
    else { newCell = groupCell + ", " + QString::number(groupAddress, 16).toUpper(); }

    query.prepare("UPDATE Nodes SET GroupSub = :newCell WHERE RealAddress = :realAddress");
    query.bindValue(":newCell", newCell);
    query.bindValue(":realAddress", realAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query:" << query.lastError().text(); }
}

void Database::setNodeFeatures(uint16_t nodeAddress, uint8_t deviceType, uint8_t ratedDuration, uint8_t emergencyFeatures, uint8_t physicalMinLvl)
{
    QSqlQuery query;
    query.prepare("UPDATE Nodes SET DeviceType = :deviceType, RatedDuration = :ratedDuration, EmergencyFeatures = :emergencyFeatures, PhysicalMinLvl = :physicalMinLvl WHERE RealAddress = :nodeAddress");
    query.bindValue(":deviceType", deviceType);
    query.bindValue(":ratedDuration", ratedDuration * 2);
    query.bindValue(":emergencyFeatures", emergencyFeatures);
    query.bindValue(":physicalMinLvl", physicalMinLvl);
    query.bindValue(":nodeAddress", nodeAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in setNodeFeatures:" << query.lastError().text(); }
}


void Database::setNodeRegister(QString nodeRegister, uint16_t nodeAddress, uint8_t value)
{
    QSqlQuery query;
    query.prepare(QString("UPDATE Nodes SET %1 = :value WHERE RealAddress = :nodeAddress").arg(nodeRegister));
    query.bindValue(":value", value);
    query.bindValue(":nodeAddress", nodeAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in setNodeRegister:" << query.lastError().text(); }
}

void Database::deleteNode(uint16_t nodeAddress)
{
    QSqlQuery query;
    query.prepare("DELETE FROM Nodes WHERE RealAddress = :nodeAddress");
    query.bindValue(":nodeAddress", nodeAddress);

    if (!query.exec()) { qDebug() << "Error executing DELETE query in deleteNode:" << query.lastError().text(); }
}

void Database::delGroup(uint16_t realAddress, uint16_t groupAddress)
{
    QSqlQuery query;

    query.prepare("SELECT GroupSub FROM Nodes WHERE RealAddress = :realAddress");
    query.bindValue(":realAddress", realAddress);

    if (!query.exec()) { return; }

    QString groupCell;
    if (query.next()) { groupCell = query.value(0).toString(); }

    QString groupAddressStr = QString::number(groupAddress, 16).toUpper();
    QStringList groups = groupCell.split(", ");

    if (!groups.contains(groupAddressStr)) { return; }

    groups.removeAll(groupAddressStr);
    QString newCell = groups.join(", ");

    query.prepare("UPDATE Nodes SET GroupSub = :newCell WHERE RealAddress = :realAddress");
    query.bindValue(":newCell", newCell);
    query.bindValue(":realAddress", realAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query:" << query.lastError().text(); }
}

void Database::setTestEnable(QString groupAddress, bool isFunctionalEnable, bool isDurationEnable)
{
    QSqlQuery query;

    query.prepare("UPDATE Test SET FunctionalEnable = :isFunctionalEnable, DurationEnable = :isDurationEnable WHERE GroupAddress = :groupAddress");
    query.bindValue(":isFunctionalEnable", isFunctionalEnable);
    query.bindValue(":isDurationEnable", isDurationEnable);
    query.bindValue(":groupAddress", groupAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query:" << query.lastError().text(); }
}

void Database::setFunctionalTest(QString groupAddress, QString functionalDays, QString functionalTime)
{
    QSqlQuery query;

    query.prepare("UPDATE Test SET FunctionalDays = :functionalDays, FunctionalTime = :functionalTime WHERE GroupAddress = :groupAddress");
    query.bindValue(":functionalDays", functionalDays);
    query.bindValue(":functionalTime", functionalTime);
    query.bindValue(":groupAddress", groupAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query:" << query.lastError().text(); }
}

void Database::setDurationTest(QString groupAddress, QString durationPeriodicity, QString durationDate, QString durationTime)
{
    QSqlQuery query;

    query.prepare("UPDATE Test SET DurationPeriodicity = :durationPeriodicity, DurationDate = :durationDate, DurationTime = :durationTime WHERE GroupAddress = :groupAddress");
    query.bindValue(":durationPeriodicity", durationPeriodicity);
    query.bindValue(":durationDate", durationDate);
    query.bindValue(":durationTime", durationTime);
    query.bindValue(":groupAddress", groupAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query:" << query.lastError().text(); }
}

QList<uint16_t> Database::getConfiguredNodes()
{
    QSqlQuery query;
    QList<uint16_t> nodeNetAddressList;
    if (!query.exec("SELECT * FROM Nodes")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    while (query.next()) {
        uint8_t subnetAddress = query.value("SubnetAddress").toUInt();
        uint8_t nodeSubnetAddress = query.value("NodeSubnetAddress").toUInt();

        uint16_t nodeNetAddress = subnetAddress * 64 + nodeSubnetAddress + 1;
        nodeNetAddressList.append(nodeNetAddress);
    }

    return nodeNetAddressList;
}


