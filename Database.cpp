#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>

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
               "RelayMode INTEGER);");



    /* **************************************************
     *                                                  *
     *                UNASSIGNED NODES                  *
     *                                                  *
     * **************************************************/
    query.exec("CREATE TABLE IF NOT EXISTS UnassignedNodes "
               "(Serial	TEXT, "
               "NetAddress INTEGER, "
               "BluetoothAddress INTEGER, "
               "InstallKey TEXT);");



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
               "LineName TEXT, "
               "MasterAddress TEXT, "
               "InstallKey TEXT, "
               "FailComCycles INTEGER, "
               "NextUnicastAddress INTEGER);");

    query.prepare("SELECT * FROM General");

    QString ip, submask, gateway;
    ip = getInterfacesConfig("address");
    submask = getInterfacesConfig("netmask");
    gateway = getInterfacesConfig("gateway");

    if (!query.exec()) { qDebug() << "Error executing SELECT query in Users:" << query.lastError().text(); }
    else {
        if (!query.next()) {

            query.prepare("INSERT INTO General (IP, Submask, Gateway, BuildingName, LineName, MasterAddress, InstallKey, FailComCycles, NextUnicastAddress) VALUES (:ip, :submask, :gateway, :buildingName, :lineName, :masterAddress, :ik, :fcc, :nua)");
            query.bindValue(":ip", ip);
            query.bindValue(":submask", submask);
            query.bindValue(":gateway", gateway);
            query.bindValue(":buildingName", "NO_NAME");
            query.bindValue(":lineName", "NO_NAME");
            query.bindValue(":masterAddress", "7C18");
            query.bindValue(":ik", "1");
            query.bindValue(":fcc", 5);
            query.bindValue(":nua", 0);

            if (!query.exec()) { qDebug() << "Error executing INSERT query in General:" << query.lastError().text(); }
        }
    }



    /* **************************************************
     *                                                  *
     *                     GROUPS                       *
     *                                                  *
     * **************************************************/
    query.exec("CREATE TABLE IF NOT EXISTS Groups "
               "(GroupAddress TEXT, "
               "GroupName TEXT, "
               "PowerOnLevel INTEGER);");

    query.prepare("SELECT * FROM Groups");

    if (!query.exec()) { qDebug() << "Error executing SELECT query in Groups:" << query.lastError().text(); }
    else {
        if (!query.next()) {
            // Antes se creaban 16 grupos por defecto -> Ahora ninguno. Se conserva el código por si acaso.
            QStringList groupAddresses = {"C010", "C011", "C012", "C013", "C014", "C015", "C016", "C017", "C018", "C019", "C01A", "C01B"};
            // QStringList groupAddresses = {};

            query.prepare("INSERT INTO Groups (GroupAddress, GroupName, PowerOnLevel) VALUES (:groupAddress, :groupName, :powerOnLevel)");

            int groupNumber = 4;
            foreach (const QString &groupAddress, groupAddresses) {
                query.bindValue(":groupAddress", groupAddress);
                query.bindValue(":groupName", "Group " + QString::number(groupNumber));
                query.bindValue(":powerOnLevel", 255);

                if (!query.exec()) { qDebug() << "Error executing INSERT query in Groups:" << query.lastError().text(); }

                groupNumber++;
            }
        }
    }



    /* **************************************************
     *                                                  *
     *                   FIXED GROUPS                   *
     *                                                  *
     * **************************************************/
    query.exec("CREATE TABLE IF NOT EXISTS FixedGroups "
               "(GroupAddress TEXT, "
               "GroupName TEXT, "
               "PowerOnLevel INTEGER);");

    query.prepare("SELECT * FROM FixedGroups");

    if (!query.exec()) { qDebug() << "Error executing SELECT query in FixedGroups:" << query.lastError().text(); }
    else {
        if (!query.next()) {
            QStringList groupAddresses = {"C000", "C001", "C002", "C003"};
            QStringList groupNames = {"(Gr0) Lighting", "(Gr1) Emergency", "(Gr2) Even", "(Gr3) Odd"};

            query.prepare("INSERT INTO FixedGroups (GroupAddress, GroupName, PowerOnLevel) VALUES (:groupAddress, :groupName, :powerOnLevel)");

            for (int i = 0; i < groupAddresses.size(); ++i) {
                query.bindValue(":groupAddress", groupAddresses[i]);
                query.bindValue(":groupName", groupNames[i]);
                query.bindValue(":powerOnLevel", 255);

                if (!query.exec()) { qDebug() << "Error executing INSERT query in FixedGroups:" << query.lastError().text(); }
            }
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
            // Antes se creaba el test de broadcast y de los 16 grupos por defecto -> Ahora solo la entrada de broadcast. Se conserva el código por si acaso.
            QStringList groupAddresses = {"FFFF", "C010", "C011", "C012", "C013", "C014", "C015", "C016", "C017", "C018", "C019", "C01A", "C01B"};
            // QStringList groupAddresses = {"FFFF"};

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



    /* **************************************************
     *                                                  *
     *                   FIXED TEST                     *
     *                                                  *
     * **************************************************/
    query.exec("CREATE TABLE IF NOT EXISTS FixedTest "
               "(GroupAddress TEXT, "
               "FunctionalEnable INTEGER, "
               "DurationEnable INTEGER, "
               "FunctionalDays TEXT, "
               "FunctionalTime TEXT, "
               "DurationPeriodicity TEXT, "
               "DurationDate TEXT, "
               "DurationTime TEXT);");

    query.prepare("SELECT * FROM FixedTest");

    if (!query.exec()) { qDebug() << "Error executing SELECT query in FixedTest:" << query.lastError().text(); }
    else {
        if (!query.next()) {
            QStringList groupAddresses = {"C000", "C001", "C002", "C003"};

            query.prepare("INSERT INTO FixedTest (GroupAddress, FunctionalEnable, DurationEnable, FunctionalDays, FunctionalTime, DurationPeriodicity, DurationDate, DurationTime) "
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

                if (!query.exec()) { qDebug() << "Error executing INSERT query in FixedTest:" << query.lastError().text(); }
            }
        }
    }



    /* **************************************************
     *                                                  *
     *                      LOG                         *
     *                                                  *
     * **************************************************/
    query.exec("CREATE TABLE IF NOT EXISTS Log "
        "(Name TEXT, "
        "Serial TEXT, "
        "BtAddress INTEGER, "
        "IP TEXT, "
        "Timestamp INTEGER, "
        "Event INTEGER, "
        "EventType TEXT);");

    query.prepare("SELECT * FROM Log");

    if (!query.exec()) { qDebug() << "Error executing SELECT query in Log:" << query.lastError().text(); }
    //query.exec("CREATE INDEX IF NOT EXISTS idx_log_timestamp ON Log(Timestamp);");
    //query.exec("CREATE INDEX IF NOT EXISTS idx_log_eventType ON Log(EventType);");
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

QStringList Database::getInterfaceParameters()
{
    QStringList list;
    QSqlQuery query;

    query.prepare("SELECT * FROM General");

    if (!query.exec()) { return list; }

    if (query.next()) {
        list.append(query.value("IP").toString());
        list.append(query.value("Submask").toString());
        list.append(query.value("Gateway").toString());
        list.append(query.value("BuildingName").toString());
        list.append(query.value("LineName").toString());
    }

    return list;
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

void Database::setGeneralData(const QString &key, const QString &value)
{
    QSqlQuery query;
    QString sql = QString("UPDATE General SET %1 = :value").arg(key);
    query.prepare(sql);
    query.bindValue(":value", value.trimmed().left(16));

    if (!query.exec()) {
        qDebug() << "Error updating" << key << "in DB:" << query.lastError().text();
    }
}

QString Database::getGeneralData(const QString &key)
{
    QSqlQuery query(QString("SELECT %1 FROM General").arg(key));
    if (query.next()) return query.value(0).toString();
    return "";
}

void Database::loadNodesFromDatabase()
{
    QSqlQuery query;
    if (!query.exec("SELECT * FROM Nodes")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    while (query.next()) {
        uint8_t subnetAddress = query.value("SubnetAddress").toUInt();
        uint8_t nodeSubnetAddress = query.value("NodeSubnetAddress").toUInt();
        QString UUIDString = query.value("UUID").toString();
        QString groupSub = query.value("GroupSub").toString();

        uint8_t UUID[16];
        convertUuidStringToByteArray(UUIDString, UUID);
        meshDevice[subnetAddress][nodeSubnetAddress].setUUID(UUID);

        uint16_t groupSubAddresses[MESH_GROUP_COUNT];
        uint8_t groupCount = convertGroupSubStringToArray(groupSub, groupSubAddresses);
        for (uint8_t i = 0; i < groupCount; i++) { meshDevice[subnetAddress][nodeSubnetAddress].setGroupSubAddress(groupSubAddresses[i]); }

        meshDevice[subnetAddress][nodeSubnetAddress].setIsConfigured(true);
        meshDevice[subnetAddress][nodeSubnetAddress].setRealAddress(query.value("RealAddress").toUInt());
        meshDevice[subnetAddress][nodeSubnetAddress].setDeviceType(query.value("DeviceType").toUInt());
    }
    polling.setConfiguredSubnets(); // polling for eth send data
}

void Database::loadTestsFromDatabase()
{
    QSqlQuery query;
    if (!query.exec("SELECT * FROM FixedTest")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

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

    if (!query.exec("SELECT * FROM Test")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

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
    if (nodeUUID) {
        for (int i = 15; i >= 0; i--) {
            nodeUUIDText += QString::asprintf("%02X", nodeUUID[i]);
        }
    } else {
        nodeUUIDText = "";
    }

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
        query.prepare("INSERT INTO Nodes (SubnetAddress, NodeSubnetAddress, RealAddress, UUID, GroupSub) VALUES (:subnetAddress, :nodeSubnetAddress, :realAddress, :uuid, :groupSub)");
    }
    else {
        query.prepare("UPDATE Nodes SET SubnetAddress = :subnetAddress, NodeSubnetAddress = :nodeSubnetAddress, RealAddress = :realAddress, GroupSub = :groupSub WHERE UUID = :uuid");
    }

    query.bindValue(":subnetAddress", subnetAddress);
    query.bindValue(":nodeSubnetAddress", nodeSubnetAddress);
    query.bindValue(":realAddress", realAddress);
    query.bindValue(":uuid", nodeUUIDText);
    query.bindValue(":groupSub", "");

    if (!query.exec()) { qDebug() << "Error executing INSERT query in setNewNode:" << query.lastError().text(); }
}

void Database::setRecoveryNode(uint8_t subnetAddress, uint8_t nodeSubnetAddress, uint16_t realAddress, uint8_t *nodeUUID)
{
    QString nodeUUIDText;
    if (nodeUUID) {
        for (int i = 0; i <= 15; i++) { // SE PROCESA EN ESTE ORDEN PORQUE SE ENVIA AL REVES DESDE EL MICRO Y NODOS
            nodeUUIDText += QString::asprintf("%02X", nodeUUID[i]);
        }
    } else {
        nodeUUIDText = "";
    }

    QSqlQuery query;

    query.prepare("INSERT INTO Nodes (SubnetAddress, NodeSubnetAddress, RealAddress, UUID, DeviceType, RelayMode) VALUES (:subnetAddress, :nodeSubnetAddress, :realAddress, :uuid, :dt, :rm)");

    query.bindValue(":subnetAddress", subnetAddress);
    query.bindValue(":nodeSubnetAddress", nodeSubnetAddress);
    query.bindValue(":realAddress", realAddress);
    query.bindValue(":uuid", nodeUUIDText);
    query.bindValue(":dt", 1);
    query.bindValue(":rm", 0);

    if (!query.exec()) { qDebug() << "Error executing INSERT query in setRecoveryNode:" << query.lastError().text(); }
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

void Database::setNodeFeatures(uint16_t nodeAddress, uint8_t deviceType, bool relayMode)
{
    QSqlQuery query;
    query.prepare("UPDATE Nodes SET DeviceType = :deviceType, RelayMode = :relayMode WHERE RealAddress = :nodeAddress");
    query.bindValue(":deviceType", deviceType);
    query.bindValue(":nodeAddress", nodeAddress);
    query.bindValue(":relayMode", relayMode);

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

void Database::deleteAllNodes()
{
    QSqlQuery query;
    query.prepare("DELETE FROM Nodes");

    if (!query.exec()) { qDebug() << "Error executing DELETE query in deleteAllNodes:" << query.lastError().text(); }
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

bool Database::deviceIsInGroup(uint16_t realAddress, uint16_t groupAddress)
{
    QSqlQuery query;
    query.prepare("SELECT GroupSub FROM Nodes WHERE RealAddress = :realAddress");
    query.bindValue(":realAddress", realAddress);

    if (!query.exec() || !query.next())
        return false;

    QString groupList = query.value(0).toString();
    QStringList groups = groupList.split(",", QString::SkipEmptyParts);

    for (QString g : groups) {
        if (g.trimmed().toUpper() == QString::number(groupAddress, 16).toUpper()) {
            return true;
        }
    }
    return false;
}

QString Database::getTests(QString groupAddress)
{
    QSqlQuery query;

    if(groupAddress == "C000" || groupAddress == "C001" || groupAddress == "C002" || groupAddress == "C003")
        query.prepare("SELECT * FROM FixedTest WHERE GroupAddress = :groupAddress");
    else
        query.prepare("SELECT * FROM Test WHERE GroupAddress = :groupAddress");

    query.bindValue(":groupAddress", groupAddress);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return ""; }

    if(query.next())
    {
        QString testString = query.value("FunctionalEnable").toString();
        testString = testString + "#" + query.value("DurationEnable").toString();
        testString = testString + "#" + query.value("FunctionalDays").toString();
        testString = testString + "#" + query.value("FunctionalTime").toString();
        testString = testString + "#" + query.value("DurationPeriodicity").toString();
        testString = testString + "#" + query.value("DurationDate").toString();
        testString = testString + "#" + query.value("DurationTime").toString();
        return testString;
    }
    else
    {
        return "0#0# #00:00#0#0000-00-00#00:00"; // Cadena con todos los datos vacíos
    }
}

void Database::setTestEnable(QString groupAddress, bool isFunctionalEnable, bool isDurationEnable)
{
    QSqlQuery query;

    if(groupAddress == "C000" || groupAddress == "C001" || groupAddress == "C002" || groupAddress == "C003")
        query.prepare("UPDATE FixedTest SET FunctionalEnable = :isFunctionalEnable, DurationEnable = :isDurationEnable WHERE GroupAddress = :groupAddress");
    else
        query.prepare("UPDATE Test SET FunctionalEnable = :isFunctionalEnable, DurationEnable = :isDurationEnable WHERE GroupAddress = :groupAddress");

    query.bindValue(":isFunctionalEnable", isFunctionalEnable);
    query.bindValue(":isDurationEnable", isDurationEnable);
    query.bindValue(":groupAddress", groupAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query:" << query.lastError().text(); return; }
}

void Database::setFunctionalTest(QString groupAddress, QString functionalDays, QString functionalTime)
{
    QSqlQuery query;

    if(groupAddress == "C000" || groupAddress == "C001" || groupAddress == "C002" || groupAddress == "C003")
        query.prepare("UPDATE FixedTest SET FunctionalDays = :functionalDays, FunctionalTime = :functionalTime WHERE GroupAddress = :groupAddress");
    else
        query.prepare("UPDATE Test SET FunctionalDays = :functionalDays, FunctionalTime = :functionalTime WHERE GroupAddress = :groupAddress");

    query.bindValue(":functionalDays", functionalDays);
    query.bindValue(":functionalTime", functionalTime);
    query.bindValue(":groupAddress", groupAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query:" << query.lastError().text(); }
}

void Database::setDurationTest(QString groupAddress, QString durationPeriodicity, QString durationDate, QString durationTime)
{
    QSqlQuery query;

    if(groupAddress == "C000" || groupAddress == "C001" || groupAddress == "C002" || groupAddress == "C003")
        query.prepare("UPDATE FixedTest SET DurationPeriodicity = :durationPeriodicity, DurationDate = :durationDate, DurationTime = :durationTime WHERE GroupAddress = :groupAddress");
    else
        query.prepare("UPDATE Test SET DurationPeriodicity = :durationPeriodicity, DurationDate = :durationDate, DurationTime = :durationTime WHERE GroupAddress = :groupAddress");

    query.bindValue(":durationPeriodicity", durationPeriodicity);
    query.bindValue(":durationDate", durationDate);
    query.bindValue(":durationTime", durationTime);
    query.bindValue(":groupAddress", groupAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query:" << query.lastError().text(); }
}

bool Database::groupExistsInTestTable(QString groupAddress)
{
    QSqlQuery query;
    if(groupAddress == "C000" || groupAddress == "C001" || groupAddress == "C002" || groupAddress == "C003")
        query.prepare("SELECT 1 FROM FixedTest WHERE GroupAddress = :groupAddress");
    else
        query.prepare("SELECT 1 FROM Test WHERE GroupAddress = :groupAddress");
    query.bindValue(":groupAddress", groupAddress);

    if (!query.exec()) {
        qDebug() << "Error checking if group exists in Test table:" << query.lastError().text();
        return false;
    }

    return query.next();
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

bool Database::isNodeInDatabase(uint16_t nodeAddress) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM Nodes WHERE address = ?");
    query.addBindValue(nodeAddress);

    if (!query.exec()) {
        qDebug() << "Error ejecutando consulta en isNodeInDatabase:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;  // Retorna true si el nodo aún está en la base de datos
    }

    return false;
}

QList<QString> Database::getConfiguredNodesAndSerialNumbers()
{
    QSqlQuery query;
    QList<QString> nodeNetAddressAndSNList;
    if (!query.exec("SELECT SubnetAddress, NodeSubnetAddress, UUID, RelayMode FROM Nodes")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    while (query.next()) {
        uint8_t subnetAddress = query.value("SubnetAddress").toUInt();
        uint8_t nodeSubnetAddress = query.value("NodeSubnetAddress").toUInt();
        uint16_t nodeNetAddress = subnetAddress * 64 + nodeSubnetAddress + 1;

        QString UUID = query.value("UUID").toString();
        QString nums = UUID.right(8);
        QString serialNumber = nums.left(2) + "." + nums.mid(2,2) + "." + nums.mid(4,2) + "." + nums.mid(6,2);

        uint8_t relayStatus = query.value("RelayMode").toUInt();

        QString nodeInfo = QString("%1#%2#%3").arg(nodeNetAddress).arg(serialNumber).arg(relayStatus);
        nodeNetAddressAndSNList.append(nodeInfo);
    }

    return nodeNetAddressAndSNList;
}

QList<QPair<QString, QString>> Database::getGroups()
{
    QSqlQuery query;
    QList<QPair<QString, QString>> groupList;
    if (!query.exec("SELECT * FROM Groups")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    while (query.next()) {
        QString groupAddress = query.value("GroupAddress").toString();
        QString groupName = query.value("GroupName").toString();

        groupList.append(qMakePair(groupAddress, groupName));
    }

    return groupList;
}

void Database::createGroup()
{
    QString newGroupAddress, newGroupName;

    QSqlQuery query;

    // Se extrae el GroupAddress del último grupo añadido
    query.prepare("SELECT GroupAddress FROM Groups ORDER BY GroupAddress DESC LIMIT 1");
    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return; }

    QString lastGroupAddress;
    if(query.next())
        lastGroupAddress = query.value("GroupAddress").toString();

    // Si no existe un último GroupAddress, damos el primer valor destinado a las direcciones de grupo
    if(lastGroupAddress.isEmpty()) {
        newGroupAddress = "C010";
        newGroupName = "Group 4";
    }
    // Si el último GroupAddress no es el máximo, obtenemos el siguiente con un incremento unitario
    else if(lastGroupAddress != "FEFF") {
        bool ok;
        uint groupAddr = lastGroupAddress.toUInt(&ok, 16);
        if (!ok) { qDebug() << "Error converting group address:" << lastGroupAddress; return; }
        groupAddr++;
        newGroupAddress = QString("%1").arg(groupAddr, 4, 16, QLatin1Char('0')).toUpper();
        newGroupName = "Group " + QString::number(groupAddr - 49167 + 3); // 49167 es la última dirección no perteneciente a grupos, 3 es un offset
    }
    // Si el último GroupAddress es el máximo, hay que buscar GroupAddress intermedios disponibles
    else {
        QSet<QString> usedAddresses;
        if (!query.exec("SELECT GroupAddress FROM Groups")) {
            qDebug() << "Error retrieving group addresses:" << query.lastError().text(); return; }

        while (query.next())
            usedAddresses.insert(query.value("GroupAddress").toString().toUpper());

        uint start = QString("C010").toUInt(nullptr, 16);
        uint end   = QString("FEFF").toUInt(nullptr, 16);
        bool found = false;

        // Buscar el primer GroupAddress no usado en ese rango
        for (uint addr = start; addr <= end; addr++) {
            QString addrStr = QString("%1").arg(addr, 4, 16, QLatin1Char('0')).toUpper();
            if (!usedAddresses.contains(addrStr)) {
                newGroupAddress = addrStr;
                newGroupName = "Group " + newGroupAddress;
                found = true;
                break;
            }
        }

        if (!found) { qDebug() << "No available GroupAddress"; return; }
    }

    // Se inserta el nuevo grupo con ese GroupAddress y el nombre del parámetro
    query.prepare("INSERT INTO Groups (GroupName, GroupAddress, PowerOnLevel) VALUES (?, ?, 255)");
    query.addBindValue(newGroupName);
    query.addBindValue(newGroupAddress);
    if (!query.exec()) { qDebug() << "Error inserting new group:" << query.lastError().text(); return; }

    createTestEntry(newGroupAddress);

    // Log entry
    LogInfo log;
    log.name = newGroupName + " [G]";
    log.serialNum = "FF.FF.FF.FF";
    log.btAddress = newGroupAddress.toUShort(nullptr, 16);
    log.devIP = getAntennaInfo(this).ip;
    log.timestamp = getAntennaInfo(this).timestamp.toSecsSinceEpoch();;
    log.event = LOG_GROUP_CREATED;
    log.eventType = "Groups";

    insertLogEvent(log);
}

void Database::createTestEntry(QString address)
{
    QSqlQuery query;

    query.prepare("INSERT INTO Test (GroupAddress, FunctionalEnable, DurationEnable, FunctionalDays, FunctionalTime, DurationPeriodicity, DurationDate, DurationTime) "
                  "VALUES (:groupAddress, :functionalEnable, :durationEnable, :functionalDays, :functionalTime, :durationPeriodicity, :durationDate, :durationTime)");

    query.bindValue(":groupAddress", address);
    query.bindValue(":functionalEnable", 0);
    query.bindValue(":durationEnable", 0);
    query.bindValue(":functionalDays", " ");
    query.bindValue(":functionalTime", "00:00");
    query.bindValue(":durationPeriodicity", "0");
    query.bindValue(":durationDate", "0000-00-00");
    query.bindValue(":durationTime", "00:00");

    if (!query.exec()) { qDebug() << "Error executing INSERT query in Test:" << query.lastError().text(); }
}

void Database::removeGroup(QString address)
{
    QString groupName = getGroupName(address);

    QSqlQuery query;
    query.prepare("DELETE FROM Groups WHERE GroupAddress = ?");
    query.addBindValue(address);

    if (!query.exec()) { qDebug() << "Error deleting group with address" << address << ":" << query.lastError().text(); return; }

    removeTestEntry(address);

    // Log entry
    LogInfo log;
    log.name = groupName + " [G]";
    log.serialNum = "FF.FF.FF.FF";
    log.btAddress = address.toUShort(nullptr, 16);
    log.devIP = getAntennaInfo(this).ip;
    log.timestamp = getAntennaInfo(this).timestamp.toSecsSinceEpoch();;
    log.event = LOG_GROUP_DELETED;
    log.eventType = "Groups";

    insertLogEvent(log);
}

void Database::removeTestEntry(QString address)
{
    QSqlQuery query;
    query.prepare("DELETE FROM Test WHERE GroupAddress = ?");
    query.addBindValue(address);

    if (!query.exec()) { qDebug() << "Error deleting test with address" << address << ":" << query.lastError().text(); }
}

bool Database::insertLogEvent(const LogInfo log)
{
    QSqlQuery query;

    query.prepare("INSERT INTO Log (Name, Serial, BtAddress, IP, Timestamp, Event, EventType) "
                  "VALUES (:name, :serial, :btAddress, :ip, :timestamp, :event, :eventType)");

    query.bindValue(":name", log.name);
    query.bindValue(":serial", log.serialNum);
    query.bindValue(":btAddress", log.btAddress);
    query.bindValue(":ip", log.devIP);
    query.bindValue(":timestamp", log.timestamp);
    query.bindValue(":event", log.event);
    query.bindValue(":eventType", log.eventType);

    if (!query.exec()) { qDebug() << "Error inserting log event:" << query.lastError().text(); }

    return true;
}

QList<QStringList> Database::getLogEvent(const QString &type, qint64 startDate, qint64 endDate)
{
    QList<QStringList> results;
    QSqlQuery query;
    QString queryStr;

    if (type.toLower() == "all") {
        queryStr = "SELECT Name, Serial, BtAddress, IP, Timestamp, Event, EventType "
                   "FROM Log WHERE Timestamp BETWEEN :start AND :end ORDER BY Timestamp DESC";
        query.prepare(queryStr);
        query.bindValue(":start", startDate);
        query.bindValue(":end", endDate);
    } else {
        queryStr = "SELECT Name, Serial, BtAddress, IP, Timestamp, Event, EventType "
                   "FROM Log WHERE EventType = :type AND Timestamp BETWEEN :start AND :end ORDER BY Timestamp DESC";
        query.prepare(queryStr);
        query.bindValue(":type", type.left(1).toUpper() + type.mid(1).toLower());  // Normalize (e.g., "fail" → "Fail")
        query.bindValue(":start", startDate);
        query.bindValue(":end", endDate);
    }

    if (!query.exec()) { qDebug() << "Error in getLogEvent:" << query.lastError().text(); return results; }

    while (query.next()) {
        QStringList row;
        row << query.value(0).toString();  
        row << query.value(1).toString(); 
        row << query.value(2).toString();  
        row << query.value(3).toString();  
        QDateTime dt = QDateTime::fromSecsSinceEpoch(query.value(4).toLongLong());
        row << dt.toString("yyyy-MM-dd HH:mm:ss"); 
        row << query.value(5).toString();  
        row << query.value(6).toString();  
        results.append(row);
    }
    return results;
}

QList<QStringList> Database::getLogEventPaged(const QString &type, qint64 startDate, qint64 endDate, int page)
{
    QList<QStringList> results;
    QSqlQuery query;
    QString queryStr;

    int pageSize = 10;
    int offset = (page - 1) * pageSize;

    int resultsCounter = 0;

    if (type.toLower() == "all") {
        queryStr = "SELECT Name, Serial, BtAddress, IP, Timestamp, Event, EventType "
                   "FROM Log WHERE Timestamp BETWEEN :start AND :end "
                   "ORDER BY Timestamp DESC LIMIT :limit OFFSET :offset";
        query.prepare(queryStr);
        query.bindValue(":start", startDate);
        query.bindValue(":end", endDate);
        query.bindValue(":limit", pageSize);
        query.bindValue(":offset", offset);
    } else {
        queryStr = "SELECT Name, Serial, BtAddress, IP, Timestamp, Event, EventType "
                   "FROM Log WHERE EventType = :type AND Timestamp BETWEEN :start AND :end "
                   "ORDER BY Timestamp DESC LIMIT :limit OFFSET :offset";
        query.prepare(queryStr);
        query.bindValue(":type", type.left(1).toUpper() + type.mid(1).toLower());  // Normalize (e.g., "fail" → "Fail")
        query.bindValue(":start", startDate);
        query.bindValue(":end", endDate);
        query.bindValue(":limit", pageSize);
        query.bindValue(":offset", offset);
    }

    if (!query.exec()) { qDebug() << "Error in getLogEventPaged:" << query.lastError().text(); return results; }

    while (query.next()) {
        resultsCounter++;
        QStringList row;
        row << query.value(0).toString();
        row << query.value(1).toString();
        row << query.value(2).toString();
        row << query.value(3).toString();
        QDateTime dt = QDateTime::fromSecsSinceEpoch(query.value(4).toLongLong());
        row << dt.toString("yyyy-MM-dd HH:mm:ss");
        row << query.value(5).toString();
        row << query.value(6).toString();
        results.append(row);
    }

    while(resultsCounter < pageSize) {
        QStringList row;
        for(int i = 0; i < 7; i++) { row << "-"; } // 7 porque hay 7 columnas en la tabla logs
        results.append(row);
        resultsCounter++;
    }

    return results;
}

QList<QStringList> Database::getAllTestLogs()
{
    QList<QStringList> results;

    QSqlQuery query;
    query.prepare("SELECT GroupAddress, FunctionalEnable, DurationEnable, FunctionalDays, FunctionalTime, DurationPeriodicity, DurationDate, DurationTime FROM Test");

    if (!query.exec()) {
        qDebug() << "Error in getAllTestLogs:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        QStringList row;
        for (int i = 0; i < 8; ++i) {
            row << query.value(i).toString();
        }
        results.append(row);
    }

    return results;
}

QList<QStringList> Database::getLastNLogEvents(int count)
{
    QList<QStringList> results;
    QSqlQuery query;

    query.prepare("SELECT Name, Serial, BtAddress, IP, Timestamp, Event "
                  "FROM Log ORDER BY Timestamp DESC LIMIT :limit");
    query.bindValue(":limit", count);

    if (!query.exec()) {
        qDebug() << "Error in getLastNLogEvents:" << query.lastError().text();
        return results;
    }

    while (query.next()) {
        QStringList row;
        row << query.value(0).toString();
        row << query.value(1).toString();
        row << query.value(2).toString();
        row << query.value(3).toString();
        QDateTime dt = QDateTime::fromSecsSinceEpoch(query.value(4).toLongLong());
        row << dt.toString("yyyy-MM-dd HH:mm:ss");
        row << query.value(5).toString();
        results.append(row);
    }

    return results;
}

int Database::getLogSize()
{
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) FROM Log")) {
        qDebug() << "Error in getLogSize:" << query.lastError().text();
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

void Database::updateRelayMode(uint16_t nodeAddress, bool enabled)
{
    QSqlQuery query;
    query.prepare("UPDATE Nodes SET RelayMode = :enabled WHERE RealAddress = :nodeAddress");
    query.bindValue(":enabled", enabled);
    query.bindValue(":nodeAddress", nodeAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in NODES" << query.lastError().text(); }
}

QString Database::getNextNodeName(uint16_t doneIts)
{
    QSqlQuery query;
    query.prepare("SELECT SubnetAddress, NodeSubnetAddress FROM Nodes ORDER BY RealAddress ASC LIMIT 1 OFFSET :offset");
    query.bindValue(":offset", doneIts);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return "Node -"; }

    if (query.next()) {
        uint8_t subnetAddress = query.value("SubnetAddress").toUInt();
        uint8_t nodeSubnetAddress = query.value("NodeSubnetAddress").toUInt();

        uint16_t nodeNetAddress = subnetAddress * 64 + nodeSubnetAddress + 1;

        return QString("Node %1").arg(nodeNetAddress);
    }
    else
        return "Node -";
}

uint16_t Database::getNextNodeAddress(uint16_t doneIts)
{
    QSqlQuery query;
    query.prepare("SELECT RealAddress FROM Nodes ORDER BY RealAddress ASC LIMIT 1 OFFSET :offset");
    query.bindValue(":offset", doneIts);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return 0x0000; }

    if (query.next()) {
        uint16_t realAddress = query.value("RealAddress").toUInt();

        return realAddress;
    }
    else
        return 0x0000;
}

uint16_t Database::getMasterRealAddress()
{
    QSqlQuery query;

    query.prepare("SELECT MasterAddress FROM General");

    if (!query.exec()) { return 0x0000; }

    if (query.next()) {
        return static_cast<uint16_t>(query.value("MasterAddress").toString().toUInt(nullptr, 16));
    }

    return 0x0000;
}

void Database::setMasterRealAddress(uint16_t newAntennaAddress)
{
    QSqlQuery query;

    QString hexString = QString("%1").arg(newAntennaAddress, 4, 16, QChar('0')).toUpper();

    query.prepare("UPDATE General SET MasterAddress = :newAntennaAddress");
    query.bindValue(":newAntennaAddress", hexString);

    if (!query.exec()) {
        qDebug() << "Failed to update MasterAddress:" << query.lastError().text();
    } else {
        if (query.numRowsAffected() == 0) {
            qDebug() << "No rows were updated. MasterAddress remains unchanged.";
        } else {
            qDebug() << "MasterAddress updated to" << hexString;
        }
    }
}

QString Database::getInstallKey()
{
    QSqlQuery query;

    query.prepare("SELECT InstallKey FROM General");

    if (!query.exec()) { return "0"; }

    if (query.next()) {
        return query.value("InstallKey").toString();
    }

    return "0";
}

void Database::setInstallKey(QString installKey)
{
    QSqlQuery query;

    query.prepare("UPDATE General SET InstallKey = :newInstallKey");
    query.bindValue(":newInstallKey", installKey);

    if (!query.exec()) {
        qDebug() << "Failed to update InstallKey:" << query.lastError().text();
    } else {
        if (query.numRowsAffected() == 0) {
            qDebug() << "No rows were updated. InstallKey remains unchanged.";
        } else {
            qDebug() << "InstallKey updated to" << installKey;
        }
    }
}

void Database::loadFailComCycles()
{
    QSqlQuery query;
    if (!query.exec("SELECT FailComCycles FROM General")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    uint8_t cycles = 5;

    if (query.next()) {
        cycles = query.value("FailComCycles").toUInt();
    }

    failComCycles = cycles;
}

void Database::updateFailComCycles(uint8_t cycles)
{
    QSqlQuery query;
    query.prepare("UPDATE General SET FailComCycles = :fcc");
    query.bindValue(":fcc", cycles);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in GENERAL" << query.lastError().text(); }
}

void Database::editGroup(QString address, QString name)
{
    QSqlQuery query;
    query.prepare("UPDATE Groups SET GroupName = :name WHERE GroupAddress = :address");
    query.bindValue(":name", name);
    query.bindValue(":address", address);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in GROUPS" << query.lastError().text(); }
}

QString Database::getGroupName(QString groupAddress)
{
    QSqlQuery query;

    if(groupAddress == "C000" || groupAddress == "C001" || groupAddress == "C002" || groupAddress == "C003")
        query.prepare("SELECT GroupName FROM FixedGroups WHERE GroupAddress = :groupAddress");
    else
        query.prepare("SELECT GroupName FROM Groups WHERE GroupAddress = :groupAddress");

    query.bindValue(":groupAddress", groupAddress);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return "Group -"; }

    if(query.next()) { return query.value("GroupName").toString(); }
    else { return "Group -"; }
}

QString Database::getGroupAdress(QString groupName)
{
    QSqlQuery query;

    if(groupName == "Lighting" || groupName == "Emergency" || groupName == "Even" || groupName == "Odd")
        query.prepare("SELECT GroupAddress FROM FixedGroups WHERE GroupName = :groupName");
    else
        query.prepare("SELECT GroupAddress FROM Groups WHERE GroupName = :groupName");

    query.bindValue(":groupName", groupName);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return "-1"; }

    if(query.next()) { return query.value("GroupAddress").toString(); }
    else { return "-1"; }
}

void Database::setPowerOnLevel(QString groupAddress, uint8_t powerOnLevel)
{
    QSqlQuery query;

    if(groupAddress == "C000" || groupAddress == "C001" || groupAddress == "C002" || groupAddress == "C003")
        query.prepare("UPDATE FixedGroups SET PowerOnLevel = :powerOnLevel WHERE GroupAddress = :groupAddress");
    else
        query.prepare("UPDATE Groups SET PowerOnLevel = :powerOnLevel WHERE GroupAddress = :groupAddress");

    query.bindValue(":powerOnLevel", powerOnLevel);
    query.bindValue(":groupAddress", groupAddress);

    if (!query.exec()) { qDebug() << "Error setting PowerOnLevel:" << query.lastError().text(); }
}

QStringList Database::getPowerOnLevel(int page)
{
    if(page < 0) { return QStringList{}; }

    QSqlQuery query;

    QStringList groupListGeneral;
    QStringList groupListPaged;

    if (!query.exec("SELECT * FROM FixedGroups")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    while (query.next()) {
        groupListGeneral.append(query.value("GroupAddress").toString() + "_" + query.value("GroupName").toString() + "_" + query.value("PowerOnLevel").toString());
    }

    if (!query.exec("SELECT * FROM Groups")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    while (query.next()) {
        groupListGeneral.append(query.value("GroupAddress").toString() + "_" + query.value("GroupName").toString() + "_" + query.value("PowerOnLevel").toString());
    }

    for(int i = page * 16 - 16; i < page * 16; i++) {
        if(i < groupListGeneral.size()) {
            groupListPaged.append(groupListGeneral[i]);
        } else {
            groupListPaged.append("-_-_-"); // empty entry
        }
    }

    return groupListPaged;
}

void Database::clearAllData()
{
    QSqlQuery query;

    // CONSTRUCCIÓN BASE DE TABLA NODES
    if (!query.exec("DELETE FROM Nodes")) { qDebug() << "Error executing DELETE query:" << query.lastError().text(); }

    // CONSTRUCCIÓN BASE DE TABLA GROUPS
    if (!query.exec("DELETE FROM Groups")) { qDebug() << "Error executing DELETE query:" << query.lastError().text(); }

    QStringList groupAddresses1 = {"C010", "C011", "C012", "C013", "C014", "C015", "C016", "C017", "C018", "C019", "C01A", "C01B", "C01C", "C01D", "C01E", "C01F"};
    query.prepare("INSERT INTO Groups (GroupAddress, GroupName, PowerOnLevel) VALUES (:groupAddress, :groupName, :powerOnLevel)");

    int groupNumber = 1;
    foreach (const QString &groupAddress, groupAddresses1) {
        query.bindValue(":groupAddress", groupAddress);
        query.bindValue(":groupName", "Group " + QString::number(groupNumber));
        query.bindValue(":powerOnLevel", 255);

        if (!query.exec()) { qDebug() << "Error executing INSERT query in Groups:" << query.lastError().text(); }

        groupNumber++;
    }

    // CONSTRUCCIÓN BASE DE TABLA TEST
    if (!query.exec("DELETE FROM Test")) { qDebug() << "Error executing DELETE query:" << query.lastError().text(); }

    QStringList groupAddresses2 = {"FFFF", "C010", "C011", "C012", "C013", "C014", "C015", "C016", "C017", "C018", "C019", "C01A", "C01B", "C01C", "C01D", "C01E", "C01F"};
    query.prepare("INSERT INTO Test (GroupAddress, FunctionalEnable, DurationEnable, FunctionalDays, FunctionalTime, DurationPeriodicity, DurationDate, DurationTime) "
                  "VALUES (:groupAddress, :functionalEnable, :durationEnable, :functionalDays, :functionalTime, :durationPeriodicity, :durationDate, :durationTime)");

    foreach (const QString &groupAddress, groupAddresses2) {
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

    // CONSTRUCCIÓN BASE DE TABLA LOG
    if (!query.exec("DELETE FROM Log")) { qDebug() << "Error executing DELETE query:" << query.lastError().text(); }

    // CONSTRUCCIÓN BASE DE TABLA FIXEDGROUPS
    if (!query.exec("DELETE FROM FixedGroups")) { qDebug() << "Error executing DELETE query:" << query.lastError().text(); }

    QStringList groupAddresses3 = {"C000", "C001", "C002", "C003"};
    QStringList groupNames = {"Lighting", "Emergency", "Even", "Odd"};

    query.prepare("INSERT INTO FixedGroups (GroupAddress, GroupName, PowerOnLevel) VALUES (:groupAddress, :groupName, :powerOnLevel)");

    for (int i = 0; i < groupAddresses3.size(); ++i) {
        query.bindValue(":groupAddress", groupAddresses3[i]);
        query.bindValue(":groupName", groupNames[i]);
        query.bindValue(":powerOnLevel", 255);

        if (!query.exec()) { qDebug() << "Error executing INSERT query in FixedGroups:" << query.lastError().text(); }
    }

    // CONSTRUCCIÓN BASE DE TABLA FIXEDTEST
    if (!query.exec("DELETE FROM FixedTest")) { qDebug() << "Error executing DELETE query:" << query.lastError().text(); }

    QStringList groupAddresses4 = {"C000", "C001", "C002", "C003"};

    query.prepare("INSERT INTO FixedTest (GroupAddress, FunctionalEnable, DurationEnable, FunctionalDays, FunctionalTime, DurationPeriodicity, DurationDate, DurationTime) "
                  "VALUES (:groupAddress, :functionalEnable, :durationEnable, :functionalDays, :functionalTime, :durationPeriodicity, :durationDate, :durationTime)");

    foreach (const QString &groupAddress, groupAddresses4) {
        query.bindValue(":groupAddress", groupAddress);
        query.bindValue(":functionalEnable", 0);
        query.bindValue(":durationEnable", 0);
        query.bindValue(":functionalDays", " ");
        query.bindValue(":functionalTime", "00:00");
        query.bindValue(":durationPeriodicity", "0");
        query.bindValue(":durationDate", "0000-00-00");
        query.bindValue(":durationTime", "00:00");

        if (!query.exec()) { qDebug() << "Error executing INSERT query in FixedTest:" << query.lastError().text(); }
    }

}

void Database::changePosition(uint8_t subnetAddress, uint8_t nodeSubnetAddress, uint16_t realAddress)
{
    QSqlQuery query;
    query.prepare("UPDATE Nodes SET SubnetAddress = :subnetAddress, NodeSubnetAddress = :nodeSubnetAddress WHERE RealAddress = :nodeAddress");
    query.bindValue(":subnetAddress", subnetAddress);
    query.bindValue(":nodeSubnetAddress", nodeSubnetAddress);
    query.bindValue(":nodeAddress", realAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in changePosition:" << query.lastError().text(); }
}

bool Database::isExistingNode(uint16_t realAddress)
{
    QSqlQuery query;
    query.prepare("SELECT RealAddress FROM Nodes WHERE RealAddress = :addr");
    query.bindValue(":addr", realAddress);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return false; }

    return query.next(); // true si al menos una fila coincide
}

ReplaceNode Database::getNodeDataForReplace(uint16_t realAddress)
{
    ReplaceNode rn = {0x00, 0x00, "", 0x00};

    QSqlQuery query;
    query.prepare("SELECT SubnetAddress, NodeSubnetAddress, GroupSub, RelayMode FROM Nodes WHERE RealAddress = :realAddress");
    query.bindValue(":realAddress", realAddress);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return rn; }

    if(query.next()) {
        rn.subnetAddress = static_cast<uint8_t>(query.value(0).toInt());
        rn.nodeSubnetAddress = static_cast<uint8_t>(query.value(1).toInt());
        rn.groupSubAddress = query.value(2).toString();
        rn.relayMode = static_cast<uint8_t>(query.value(3).toInt());

        return rn;
    }
    else
        return rn;
}

void Database::setNodeDataForReplace(ReplaceNode replaceNode, uint16_t realAddress)
{
    QSqlQuery query;
    query.prepare("UPDATE Nodes SET SubnetAddress = :sa, NodeSubnetAddress = :nsa, GroupSub = :gs, RelayMode = :rm WHERE RealAddress = :realAddress");
    query.bindValue(":sa", replaceNode.subnetAddress);
    query.bindValue(":nsa", replaceNode.nodeSubnetAddress);
    query.bindValue(":gs", ""); // groupSub se carga con la respuesta del micro
    query.bindValue(":rm", 0); // relayMode se carga con la respuesta del micro
    query.bindValue(":realAddress", realAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in Nodes:" << query.lastError().text(); }
}

uint16_t Database::getNodeNetAddressForReplace(uint16_t realAddress)
{
    QSqlQuery query;
    query.prepare("SELECT SubnetAddress, NodeSubnetAddress FROM Nodes WHERE RealAddress = :realAddress");
    query.bindValue(":realAddress", realAddress);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return 0; }

    if(query.next()) {
        uint8_t subnetAddress = static_cast<uint8_t>(query.value(0).toInt());
        uint8_t nodeSubnetAddress = static_cast<uint8_t>(query.value(1).toInt());

        return subnetAddress * 64 + nodeSubnetAddress + 1;
    }
    else
        return 0;
}

uint16_t Database::getNextUnicastAddress()
{
    QSqlQuery query;

    query.prepare("SELECT NextUnicastAddress FROM General");

    if (!query.exec()) {
        qDebug() << "Error ejecutando SELECT en getNextUnicastAddress:" << query.lastError().text();
        return 0;
    }

    if (query.next()) {
        return static_cast<uint16_t>(query.value(0).toInt());
    } else {
        qDebug() << "No se encontró NextUnicastAddress";
        return 0;
    }
}

void Database::updateNextUnicastAddress(uint16_t nextUnicastAddress)
{
    uint16_t actualNextUnicastAddress = getNextUnicastAddress();

    if(actualNextUnicastAddress < nextUnicastAddress) {
        QSqlQuery query;
        query.prepare("UPDATE General SET NextUnicastAddress = :nua");
        query.bindValue(":nua", nextUnicastAddress);

        if (!query.exec()) { qDebug() << "Error executing UPDATE query in General:" << query.lastError().text(); }
    }
}

QList<uint16_t> Database::getAddressesDescForGlobalRemove() {
    QList<uint16_t> addresses;

    QSqlQuery query;
    query.prepare("SELECT RealAddress FROM Nodes ORDER BY RealAddress DESC");

    if (!query.exec()) {
        qDebug() << "Error executing SELECT query (RealAddress DESC):" << query.lastError().text();
        return addresses;
    }

    while (query.next()) {
        addresses.append(static_cast<uint16_t>(query.value(0).toUInt()));
    }

    return addresses;
}

bool Database::addUnassignedNode(QString serial)
{
    QSqlQuery query;

    QString s = serial;                 // "12.34.56.78"
    s.remove(".");                      // "12345678"

    // --- Comprobar si ya existe en Nodes ---
    query.prepare("SELECT UUID FROM Nodes WHERE UUID LIKE :uuid");
    query.bindValue(":uuid", "%" + s);

    if (!query.exec()) {
        qDebug() << "Error ejecutando SELECT en addUnassignedNode (I):" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        qDebug() << "Ya está registrado ese serial como nodo asignado";
        return false;
    }

    // --- Comprobar si ya existe en UnassignedNodes ---
    query.prepare("SELECT Serial FROM UnassignedNodes WHERE Serial = :serial");
    query.bindValue(":serial", serial);

    if (!query.exec()) {
        qDebug() << "Error ejecutando SELECT en addUnassignedNode (II):" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        qDebug() << "Ya está registrado ese serial como nodo no asignado";
        return false;
    }

    // --- Insertar ---
    query.prepare("INSERT INTO UnassignedNodes (Serial) VALUES (:serial)");
    query.bindValue(":serial", serial);

    if (!query.exec()) {
        qDebug() << "Error ejecutando INSERT en addUnassignedNode:" << query.lastError().text();
        return false;
    }

    return true;
}

uint16_t Database::getUnassignedNodesCount()
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM UnassignedNodes");

    if (!query.exec()) {
        qDebug() << "Error executing SELECT query in getUnassignedNodesCount:" << query.lastError().text();
        return 0;
    }

    int count = 0;
    if (query.next()) { count = query.value(0).toInt(); }

    return count;
}

QStringList Database::getUnassignedNodesPaged(uint16_t page)
{
    if(page < 1) { return QStringList{}; }

    QSqlQuery query;

    QStringList unassignedNodesGeneral;
    QStringList unassignedNodesPaged;

    if (!query.exec("SELECT * FROM UnassignedNodes")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    while (query.next()) {
        QString netAddress = query.value("NetAddress").toString();
        QString bluetoothAddress = query.value("BluetoothAddress").toString();
        QString installKey = query.value("InstallKey").toString();
        unassignedNodesGeneral.append(query.value("Serial").toString() + "_" + (netAddress == "" ? "-" : netAddress) + "_" + (bluetoothAddress == "" ? "-" : bluetoothAddress) + "_" + (installKey == "" ? "-" : installKey));
    }

    for(int i = page * 16 - 16; i < page * 16; i++) {
        if(i < unassignedNodesGeneral.size()) {
            unassignedNodesPaged.append(unassignedNodesGeneral[i]);
        } else {
            unassignedNodesPaged.append("-_-_-_-"); // empty entry
        }
    }

    return unassignedNodesPaged;
}

QList<UnassignedNode> Database::getUnassignedNodes()
{
    QSqlQuery query;

    QList<UnassignedNode> unassignedNodes;

    if (!query.exec("SELECT * FROM UnassignedNodes")) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); }

    while (query.next()) {
        QString serial = query.value("Serial").toString();
        uint16_t netAddress = query.value("NetAddress").toUInt();
        uint16_t bluetoothAddress = query.value("BluetoothAddress").toUInt();
        QString installKey = query.value("InstallKey").toString();
        unassignedNodes.append(UnassignedNode{serial, netAddress, bluetoothAddress, installKey});
    }

    return unassignedNodes;
}

void Database::clearUnassignedNodes()
{
    QSqlQuery query;

    if (!query.exec("DELETE FROM UnassignedNodes")) {
        qDebug() << "Error deleting rows:" << query.lastError().text();
    }
}

void Database::clearPartialUnassignedNodes()
{
    QSqlQuery query;

    if (!query.exec("UPDATE UnassignedNodes SET NetAddress = NULL, BluetoothAddress = NULL, InstallKey = NULL")) {
        qDebug() << "Error clearing UnassignedNodes:" << query.lastError().text();
    }
}

uint16_t Database::getMayorUnicastAddressOfUnassignedNodes()
{
    QSqlQuery query;

    if (!query.exec("SELECT MAX(BluetoothAddress) FROM UnassignedNodes")) {
        qDebug() << "Error executing MAX query:" << query.lastError().text();
        return 0;
    }

    if (query.next()) {
        QVariant value = query.value(0);
        if (!value.isNull())
            return static_cast<uint16_t>(value.toUInt()); // Si hay unassignedNodes, devuelve la mayor de las direcciones
        else
            return 0; // Si no hay unassignedNodes, devuelve 0
    }

    return 0;
}

void Database::addNodeByAssignment(QString serial, const uint8_t uuid[16], uint8_t devType)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM UnassignedNodes WHERE Serial = :serial");
    query.bindValue(":serial", serial);

    if (!query.exec()) {
        qDebug() << "Error ejecutando SELECT query:" << query.lastError().text();
        return;
    }

    uint16_t netAddress, bluetoothAddress;

    if(query.next()) {
        netAddress = query.value("NetAddress").toUInt();
        bluetoothAddress = query.value("BluetoothAddress").toUInt();
    }
    else { return; }

    query.prepare("INSERT INTO Nodes (SubnetAddress, NodeSubnetAddress, RealAddress, UUID, GroupSub, DeviceType, RelayMode) VALUES (:subnetAddress, :nodeSubnetAddress, :realAddress, :uuid, :groupSub, :deviceType, :relayMode)");
    query.bindValue(":subnetAddress", (netAddress - 1) / 64);
    query.bindValue(":nodeSubnetAddress", (netAddress - 1) % 64);
    query.bindValue(":realAddress", bluetoothAddress);

    QString uuidStr;
    for (int i = 0; i < 16; ++i)
        uuidStr += QString("%1").arg(uuid[i], 2, 16, QChar('0')).toUpper();
    query.bindValue(":uuid", uuidStr);

    QString groupStr;
    if (devType == 0x01) {
        groupStr = QString("C001, ") + (netAddress % 2 == 0 ? "C002" : "C003");
    }
    else if (devType == 0x06) {
        groupStr = "C000";
    }
    else {
        groupStr = QString("C001, ") + (netAddress % 2 == 0 ? "C002" : "C003");
    }
    query.bindValue(":groupSub", groupStr);

    query.bindValue(":deviceType", devType);
    query.bindValue(":relayMode", true);

    if (!query.exec()) {
        qDebug() << "Error ejecutando INSERT query:" << query.lastError().text();
    }
}

bool Database::delUnassignedNode(QString serial)
{
    QSqlQuery query;
    query.prepare("DELETE FROM UnassignedNodes WHERE Serial = :serial");
    query.bindValue(":serial", serial);

    if (!query.exec()) {
        qDebug() << "Error ejecutando DELETE query:" << query.lastError().text();
        return false;
    }

    return true;
}

bool Database::doAutoAssignment()
{
    QSqlQuery q;

    QVector<uint16_t> netAddresses;

    // Obtener direcciones ocupadas
    if (!q.exec("SELECT SubnetAddress, NodeSubnetAddress FROM Nodes")) {
        qDebug() << q.lastError().text();
        return false;
    }

    while (q.next()) {
        int sub = q.value(0).toInt();
        int node = q.value(1).toInt();
        int net = sub * 64 + node + 1;
        netAddresses.append(net);
    }

    std::sort(netAddresses.begin(), netAddresses.end());

    // Conteo de nodos sin asignar
    if (!q.exec("SELECT count(*) FROM UnassignedNodes")) {
        qDebug() << q.lastError().text();
        return false;
    }

    int nodesForAddressing = 0;
    if (q.next()) { nodesForAddressing = q.value(0).toInt(); }

    if(nodesForAddressing == 0) { return false; }

    // Encontrar direcciones libres (Net Address)
    QVector<uint16_t> freeNetAddresses;
    int buscado = 1;
    int idx = 0;

    while (freeNetAddresses.size() < nodesForAddressing) {
        if (idx < netAddresses.size() && netAddresses[idx] == buscado) {
            idx++;
        } else {
            freeNetAddresses.append(buscado);
        }
        buscado++;
    }

    // Encontrar la primera dirección a usar (Bluetooth Address)
    uint16_t nextUnicastAddress = getNextUnicastAddress() + 1;

    // Encontrar la installkey
    QString installKey = getInstallKey();
    installKey = installKey.size() == 32 ? "16" : installKey;

    // Obtener seriales en orden
    QVector<QString> seriales;

    if (!q.exec("SELECT Serial FROM UnassignedNodes")) {
        qDebug() << q.lastError().text();
        return false;
    }

    while (q.next())
        seriales.append(q.value(0).toString());

    // Reescribir todas las NetAddress
    q.prepare("UPDATE UnassignedNodes SET NetAddress = :na, BluetoothAddress = :ba, InstallKey = :ik WHERE Serial = :serial");

    for (int i = 0; i < seriales.size(); i++) {
        q.bindValue(":na", freeNetAddresses[i]);
        q.bindValue(":ba", nextUnicastAddress);
        q.bindValue(":ik", installKey);
        q.bindValue(":serial", seriales[i]);

        if (!q.exec()) {
            qDebug() << q.lastError().text();
            return false;
        } else {
            nextUnicastAddress++;
        }
    }

    return true;
}

bool Database::allNodesHaveAutoAssignment()
{
    QSqlQuery q;

    // Conteo de nodos sin asignar
    if (!q.exec("SELECT count(*) FROM UnassignedNodes WHERE "
            "NetAddress IS NULL OR NetAddress = '' "
            "OR BluetoothAddress IS NULL OR BluetoothAddress = '' "
            "OR InstallKey IS NULL OR InstallKey = ''")) {
        qDebug() << q.lastError().text();
        return false;
    }

    int missing = 0;
    if (q.next())
        missing = q.value(0).toInt();

    return (missing == 0);
}
