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
               "RatedDuration INTEGER, "
               "EmergencyFeatures INTEGER, "
               "PhysicalMinLvl INTEGER, "
               "RelayMode INTEGER, "
               "FatherRealAddress INTEGER, "
               "NetIdx INTEGER, "
               "NumElem INTEGER, "
               "DevKey TEXT);");



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
               "NetKey TEXT, "
               "FailComCycles INTEGER);");

    query.prepare("SELECT * FROM General");

    QString ip, submask, gateway;
    ip = getInterfacesConfig("address");
    submask = getInterfacesConfig("netmask");
    gateway = getInterfacesConfig("gateway");

    if (!query.exec()) { qDebug() << "Error executing SELECT query in Users:" << query.lastError().text(); }
    else {
        if (!query.next()) {

            query.prepare("INSERT INTO General (IP, Submask, Gateway, BuildingName, LineName, MasterAddress, NetKey, FailComCycles) VALUES (:ip, :submask, :gateway, :buildingName, :lineName, :masterAddress, :nk, :fcc)");
            query.bindValue(":ip", ip);
            query.bindValue(":submask", submask);
            query.bindValue(":gateway", gateway);
            query.bindValue(":buildingName", "NO_NAME");
            query.bindValue(":lineName", "NO_NAME");
            query.bindValue(":masterAddress", "7C18");
            query.bindValue(":nk", "1");
            query.bindValue(":fcc", 5);

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
            QStringList groupAddresses = {"C010", "C011", "C012", "C013", "C014", "C015", "C016", "C017", "C018", "C019", "C01A", "C01B", "C01C", "C01D", "C01E", "C01F"};
            // QStringList groupAddresses = {};

            query.prepare("INSERT INTO Groups (GroupAddress, GroupName, PowerOnLevel) VALUES (:groupAddress, :groupName, :powerOnLevel)");

            int groupNumber = 1;
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
            QStringList groupNames = {"Lighting", "Emergency", "Even", "Odd"};

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
            QStringList groupAddresses = {"FFFF", "C010", "C011", "C012", "C013", "C014", "C015", "C016", "C017", "C018", "C019", "C01A", "C01B", "C01C", "C01D", "C01E", "C01F"};
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
        meshDevice[subnetAddress][nodeSubnetAddress].setRatedDuration(query.value("RatedDuration").toUInt());
        meshDevice[subnetAddress][nodeSubnetAddress].setEmergencyFeatures(query.value("EmergencyFeatures").toUInt());
        meshDevice[subnetAddress][nodeSubnetAddress].setPhysicalMinLvl(query.value("PhysicalMinLvl").toUInt());
    }
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

void Database::setNewNode(uint8_t subnetAddress, uint8_t nodeSubnetAddress, uint16_t realAddress, uint8_t *nodeUUID, uint16_t fatherRealAddress)
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
        query.prepare("INSERT INTO Nodes (SubnetAddress, NodeSubnetAddress, RealAddress, UUID, GroupSub, FatherRealAddress) VALUES (:subnetAddress, :nodeSubnetAddress, :realAddress, :uuid, :groupSub, :fatherRealAddress)");
    }
    else {
        query.prepare("UPDATE Nodes SET SubnetAddress = :subnetAddress, NodeSubnetAddress = :nodeSubnetAddress, RealAddress = :realAddress, GroupSub = :groupSub, FatherRealAddress = :fatherRealAddress WHERE UUID = :uuid");
    }

    query.bindValue(":subnetAddress", subnetAddress);
    query.bindValue(":nodeSubnetAddress", nodeSubnetAddress);
    query.bindValue(":realAddress", realAddress);
    query.bindValue(":uuid", nodeUUIDText);
    query.bindValue(":groupSub", "");
    query.bindValue(":fatherRealAddress", fatherRealAddress);

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

    query.prepare("INSERT INTO Nodes (SubnetAddress, NodeSubnetAddress, RealAddress, UUID, DeviceType, RatedDuration, EmergencyFeatures, PhysicalMinLvl, RelayMode, FatherRealAddress) VALUES (:subnetAddress, :nodeSubnetAddress, :realAddress, :uuid, :dt, :rd, :ef, :pml, :rm, :fra)");

    query.bindValue(":subnetAddress", subnetAddress);
    query.bindValue(":nodeSubnetAddress", nodeSubnetAddress);
    query.bindValue(":realAddress", realAddress);
    query.bindValue(":uuid", nodeUUIDText);
    query.bindValue(":dt", 1);
    query.bindValue(":rd", 60);
    query.bindValue(":ef", 143);
    query.bindValue(":pml", 254);
    query.bindValue(":rm", 0);
    query.bindValue(":fra", getMasterRealAddress());

    if (!query.exec()) { qDebug() << "Error executing INSERT query in setRecoveryNode:" << query.lastError().text(); }
}

void Database::setFatherRealAddress(uint16_t nodeAddress, uint16_t fatherRealAddress)
{
    QSqlQuery query;
    query.prepare("UPDATE Nodes SET FatherRealAddress = :fra WHERE RealAddress = :nodeAddress");
    query.bindValue(":fra", fatherRealAddress);
    query.bindValue(":nodeAddress", nodeAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in setFatherRealAddress:" << query.lastError().text(); }
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

void Database::setNodeFeatures(uint16_t nodeAddress, uint8_t deviceType, uint8_t ratedDuration, uint8_t emergencyFeatures, uint8_t physicalMinLvl, bool relayMode)
{
    QSqlQuery query;
    query.prepare("UPDATE Nodes SET DeviceType = :deviceType, RatedDuration = :ratedDuration, EmergencyFeatures = :emergencyFeatures, PhysicalMinLvl = :physicalMinLvl, RelayMode = :relayMode WHERE RealAddress = :nodeAddress");
    query.bindValue(":deviceType", deviceType);
    query.bindValue(":ratedDuration", ratedDuration * 2);
    query.bindValue(":emergencyFeatures", emergencyFeatures);
    query.bindValue(":physicalMinLvl", physicalMinLvl);
    query.bindValue(":nodeAddress", nodeAddress);
    query.bindValue(":relayMode", relayMode);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in setNodeFeatures:" << query.lastError().text(); }
}

void Database::setExtraFeatures(uint16_t nodeAddress, uint16_t net_idx, uint8_t num_elem, uint8_t* dev_key)
{
    QString nodeDevKeyText;
    if (dev_key) {
        for (int i = 0; i < 16; i++) {
            nodeDevKeyText += QString::asprintf("%02X", dev_key[i]);
        }
    } else {
        nodeDevKeyText = "";
    }

    QSqlQuery query;
    query.prepare("UPDATE Nodes SET NetIdx = :ni, NumElem = :ne, DevKey = :dk WHERE RealAddress = :nodeAddress");
    query.bindValue(":ni", net_idx);
    query.bindValue(":ne", num_elem);
    query.bindValue(":dk", nodeDevKeyText);
    query.bindValue(":nodeAddress", nodeAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in setExtraFeatures:" << query.lastError().text(); }
}

QString Database::getDevKey(uint16_t nodeAddress)
{
    QSqlQuery query;

    query.prepare("SELECT DevKey FROM Nodes WHERE RealAddress = :nodeAddress");
    query.bindValue(":nodeAddress", nodeAddress);

    if (!query.exec()) {
        qDebug() << "Error ejecutando SELECT en getDevKey:" << query.lastError().text();
        return QString();
    }

    if (query.next()) {
        QString devKeyText = query.value(0).toString().trimmed();
        if (devKeyText.length() != 32) {
            qDebug() << "DevKey inválida (longitud incorrecta):" << devKeyText;
            return QString();
        }
        return devKeyText;
    } else {
        qDebug() << "No se encontró DevKey para la dirección" << nodeAddress;
        return QString();
    }
}

void Database::addNode(uint16_t nodeAddress)
{
    QSqlQuery query;
    query.prepare("INSERT INTO Nodes (RealAddress) VALUES (:nodeAddress)");
    query.bindValue(":nodeAddress", nodeAddress);

    if (!query.exec()) {
        qDebug() << "Error executing INSERT query in addNode:"
                 << query.lastError().text();
    }
}

void Database::addOrUpdateNode(
    uint8_t subnetAddress,
    uint8_t nodeSubnetAddress,
    uint16_t realAddress,
    const QString &uuid,
    const QString &groupSub,
    uint8_t deviceType,
    uint8_t ratedDuration,
    uint8_t emergencyFeatures,
    uint8_t physicalMinLvl,
    bool relayMode,
    uint16_t fatherRealAddress
)
{
    // Abre la BD, si no está abierta.
    if (!openDatabase()) {
        qDebug() << "Error opening DB in addOrUpdateNode()";
        return;
    }

    // 1. Verificar si YA existe un registro con el mismo RealAddress.
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM Nodes WHERE RealAddress = :rAddr");
    query.bindValue(":rAddr", static_cast<int>(realAddress));

    if (!query.exec()) {
        qDebug() << "Error SELECT in addOrUpdateNode:" << query.lastError().text();
        return;
    }

    bool exists = false;
    if (query.next()) {
        exists = (query.value(0).toInt() > 0);  
    }

    // 2. Si NO existe → INSERT
    //    Si SÍ existe → UPDATE
    if (!exists) {
        query.prepare(
         "INSERT INTO Nodes ("
         "   SubnetAddress, NodeSubnetAddress, RealAddress, UUID, GroupSub, "
         "   DeviceType, RatedDuration, EmergencyFeatures, PhysicalMinLvl, RelayMode, FatherRealAddress"
         ") VALUES ("
         "   :subnetAddress, :nodeSubnetAddress, :realAddress, :uuid, :groupSub, "
         "   :deviceType, :ratedDuration, :emergencyFeatures, :physicalMinLvl, :relayMode, :fatherRealAddress"
         ")"
        );
        qDebug() << "[DB] Insertando nodo nuevo (RealAddress:" << realAddress << ")";
    } else {
        query.prepare(
         "UPDATE Nodes SET "
         "   SubnetAddress = :subnetAddress, "
         "   NodeSubnetAddress = :nodeSubnetAddress, "
         "   UUID = :uuid, "
         "   GroupSub = :groupSub, "
         "   DeviceType = :deviceType, "
         "   RatedDuration = :ratedDuration, "
         "   EmergencyFeatures = :emergencyFeatures, "
         "   PhysicalMinLvl = :physicalMinLvl, "
         "   RelayMode = :relayMode, "
         "   FatherRealAddress = :fatherRealAddress"
         "WHERE RealAddress = :realAddress"
        );
        qDebug() << "[DB] Actualizando nodo existente (RealAddress:" << realAddress << ")";
    }

    // 3. Vincular todos los valores
    query.bindValue(":subnetAddress",     static_cast<int>(subnetAddress));
    query.bindValue(":nodeSubnetAddress", static_cast<int>(nodeSubnetAddress));
    query.bindValue(":realAddress",       static_cast<int>(realAddress));
    query.bindValue(":uuid",             uuid);
    query.bindValue(":groupSub",         groupSub);
    query.bindValue(":deviceType",       static_cast<int>(deviceType));
    query.bindValue(":ratedDuration",    static_cast<int>(ratedDuration));
    query.bindValue(":emergencyFeatures",static_cast<int>(emergencyFeatures));
    query.bindValue(":physicalMinLvl",   static_cast<int>(physicalMinLvl));
    query.bindValue(":relayMode",        relayMode);
    query.bindValue(":fatherRealAddress",fatherRealAddress);

    // 4. Ejecutar la sentencia SQL
    if (!query.exec()) {
        qDebug() << "[DB] Error en INSERT/UPDATE addOrUpdateNode:" << query.lastError().text();
    } else {
        if (!exists) {
            qDebug() << "[DB] Nodo insertado correctamente en la tabla Nodes.";
        } else {
            qDebug() << "[DB] Nodo actualizado correctamente en la tabla Nodes.";
        }
    }
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

QList<QPair<uint16_t, uint16_t>> Database::getDependentNodesList(uint16_t realAddress)
{
    QList<QPair<uint16_t, uint16_t>> result;

    // Obtener hijos directos de este padre
    QSqlQuery query;
    query.prepare("SELECT SubnetAddress, NodeSubnetAddress, RealAddress FROM Nodes WHERE FatherRealAddress = :realAddress");
    query.bindValue(":realAddress", realAddress);

    if (!query.exec()) { qWarning() << "Error ejecutando query:" << query.lastError().text(); return result; }

    while (query.next()) {
        int subnetAddress = query.value(0).toInt();
        int nodeSubnetAddress = query.value(1).toInt();
        uint16_t childNumber = subnetAddress * 64 + nodeSubnetAddress + 1;
        uint16_t childRealAddress = static_cast<uint16_t>(query.value(2).toInt());

        result.append(qMakePair(childNumber, childRealAddress));

        // Recursión: obtener todos los descendientes de este hijo
        QList<QPair<uint16_t, uint16_t>> childDescendants = getDependentNodesList(childRealAddress);
        result.append(childDescendants);
    }

    return result;
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
        newGroupName = "Group 1";
    }
    // Si el último GroupAddress no es el máximo, obtenemos el siguiente con un incremento unitario
    else if(lastGroupAddress != "FEFF") {
        bool ok;
        uint groupAddr = lastGroupAddress.toUInt(&ok, 16);
        if (!ok) { qDebug() << "Error converting group address:" << lastGroupAddress; return; }
        groupAddr++;
        newGroupAddress = QString("%1").arg(groupAddr, 4, 16, QLatin1Char('0')).toUpper();
        newGroupName = "Group " + QString::number(groupAddr - 49167); // 49167 es la última dirección no perteneciente a grupos
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

void Database::readNodesForTree()
{
    nodesByRealAddress = {};
    childrenMap = {};

    QSqlQuery query("SELECT SubnetAddress, NodeSubnetAddress, RealAddress, UUID, FatherRealAddress FROM Nodes");

    while (query.next()) {
        NodeInfo node;
        node.subnetAddress = static_cast<uint8_t>(query.value(0).toInt());
        node.nodeSubnetAddress = static_cast<uint8_t>(query.value(1).toInt());
        node.realAddress = static_cast<uint16_t>(query.value(2).toInt());
        QString nums = query.value(3).toString().right(8);
        node.serialNumber = nums.left(2) + "." + nums.mid(2,2) + "." + nums.mid(4,2) + "." + nums.mid(6,2);
        uint16_t fatherRealAddress = static_cast<uint16_t>(query.value(4).toInt());
        node.fatherRealAddress = fatherRealAddress > 31767 ? 0xC00F : fatherRealAddress; // si el padre es la antena, seteamos la dirección del grupo de antenas

        nodesByRealAddress[node.realAddress] = node;
        childrenMap.insert(node.fatherRealAddress, node.realAddress);
    }
}

void Database::updateRelayMode(uint16_t nodeAddress, bool enabled)
{
    QSqlQuery query;
    query.prepare("UPDATE Nodes SET RelayMode = :enabled WHERE RealAddress = :nodeAddress");
    query.bindValue(":enabled", enabled);
    query.bindValue(":nodeAddress", nodeAddress);

    if (!query.exec()) { qDebug() << "Error executing UPDATE query in NODES" << query.lastError().text(); }
}

uint16_t Database::getFatherRealAddress(uint16_t nodeAddress)
{
    QSqlQuery query;
    query.prepare("SELECT FatherRealAddress FROM Nodes WHERE RealAddress = :nodeAddress");
    query.bindValue(":nodeAddress", nodeAddress);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return 0xC00F; }

    if(query.next())
        return query.value("FatherRealAddress").toUInt();
    else
        return 0xC00F;
}

int Database::getCountOfDirectChildren(uint16_t nodeAddress)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM Nodes WHERE FatherRealAddress = :fatherNodeAddress");
    query.bindValue(":fatherNodeAddress", nodeAddress);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return 0; }

    if (query.next())
        return query.value(0).toInt();
    else
        return 0;
}

QList<uint16_t> Database::getChildrenRealAddresses(uint16_t nodeAddress)
{
    QList<uint16_t> childrenRealAddresses;

    QSqlQuery query;
    query.prepare("SELECT RealAddress FROM Nodes WHERE FatherRealAddress = :fra");
    query.bindValue(":fra", nodeAddress);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return childrenRealAddresses; }

    while (query.next()) {
        childrenRealAddresses.append(static_cast<uint16_t>(query.value("RealAddress").toUInt(nullptr)));
    }

    return childrenRealAddresses;
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
        qDebug() << "MasterAddress updated to" << hexString;
    }
}

QString Database::getNetKey()
{
    QSqlQuery query;

    query.prepare("SELECT NetKey FROM General");

    if (!query.exec()) { return "0"; }

    if (query.next()) {
        return query.value("NetKey").toString();
    }

    return "0";
}

void Database::setNetKey(QString netKey)
{
    QSqlQuery query;

    query.prepare("UPDATE General SET NetKey = :newNetKey");
    query.bindValue(":newNetKey", netKey);

    if (!query.exec()) {
        qDebug() << "Failed to update NetKey:" << query.lastError().text();
    } else {
        qDebug() << "NetKey updated to" << netKey;
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
    ReplaceNode rn = {0x00, 0x00, "", 0x00, 0x0000};

    QSqlQuery query;
    query.prepare("SELECT SubnetAddress, NodeSubnetAddress, GroupSub, RelayMode, FatherRealAddress FROM Nodes WHERE RealAddress = :realAddress");
    query.bindValue(":realAddress", realAddress);

    if (!query.exec()) { qDebug() << "Error executing SELECT query:" << query.lastError().text(); return rn; }

    if(query.next()) {
        rn.subnetAddress = static_cast<uint8_t>(query.value(0).toInt());
        rn.nodeSubnetAddress = static_cast<uint8_t>(query.value(1).toInt());
        rn.groupSubAddress = query.value(2).toString();
        rn.relayMode = static_cast<uint8_t>(query.value(3).toInt());
        rn.fatherRealAddress = static_cast<uint16_t>(query.value(4).toInt());

        return rn;
    }
    else
        return rn;
}

void Database::setNodeDataForReplace(ReplaceNode replaceNode, uint16_t realAddress)
{
    QSqlQuery query;
    query.prepare("UPDATE Nodes SET SubnetAddress = :sa, NodeSubnetAddress = :nsa, GroupSub = :gs, RelayMode = :rm, FatherRealAddress = :fra WHERE RealAddress = :realAddress");
    query.bindValue(":sa", replaceNode.subnetAddress);
    query.bindValue(":nsa", replaceNode.nodeSubnetAddress);
    query.bindValue(":gs", ""); // groupSub se carga con la respuesta del micro
    query.bindValue(":rm", 0); // relayMode se carga con la respuesta del micro
    query.bindValue(":fra", replaceNode.fatherRealAddress);
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
