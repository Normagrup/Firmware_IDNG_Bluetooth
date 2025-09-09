#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include "structures.h"
#include "log.h"

class Database : public QObject
{
    Q_OBJECT
public:
    explicit Database(QObject *parent = nullptr);

    void initDatabase(void);

    bool openDatabase(void);
    void closeDatabase(void);

    uint8_t verifyLoginParameters(QString loginParameters);
    QStringList getLoginParameters(void);
    QStringList getInterfaceParameters(void);
    void setInterfaceParameters(QStringList interfaceParameters);
    void loadNodesFromDatabase(void);
    void loadTestsFromDatabase(void);

    void setNewNode(uint8_t subnetAddress, uint8_t nodeSubnetAddress, uint16_t realAddress, uint8_t* nodeUUID, uint16_t fatherRealAddress);
    void setGroup(uint16_t realAddress, uint16_t groupAddress);
    void setNodeFeatures(uint16_t nodeAddress, uint8_t deviceType, uint8_t ratedDuration, uint8_t emergencyFeatures, uint8_t physicalMinLvl, bool relayMode);
    void setExtraFeatures(uint16_t nodeAddress, uint16_t net_idx, uint8_t num_elem, uint8_t* dev_key);
    QString getDevKey(uint16_t nodeAddress);
    QString getUUID(uint16_t nodeAddress);
    void setNodeRegister(QString nodeRegister, uint16_t nodeAddress, uint8_t value);

    bool isNodeInDatabase(uint16_t nodeAddress);
    void deleteNode(uint16_t nodeAddress);
    void deleteAllNodes(void);
    void delGroup(uint16_t realAddress, uint16_t groupAddress);
    bool deviceIsInGroup(uint16_t realAddress, uint16_t groupAddress);

    QString getTests(QString groupAddress);
    void setTestEnable(QString groupAddress, bool isFunctionalEnable, bool isDurationEnable);
    void setFunctionalTest(QString groupAddress, QString functionalDays, QString functionalTime);
    void setDurationTest(QString groupAddress, QString durationPeriodicity, QString durationDate, QString durationTime);

    QList<uint16_t> getConfiguredNodes(void);
    QList<QString> getConfiguredNodesAndSerialNumbers(void);
    QList<QPair<uint16_t, uint16_t>> getDependentNodesList(uint16_t realAddress);

    QList<QPair<QString, QString>> getGroups(void);
    void createGroup();
    void createTestEntry(QString address);
    void removeGroup(QString address);
    void removeTestEntry(QString address);
    void editGroup(QString address, QString name);
    QString getGroupName(QString groupAddress);
    void setPowerOnLevel(QString groupAddress, uint8_t powerOnLevel);
    QStringList getPowerOnLevel(int page);

    void clearAllData(void);
    void addNode(uint16_t nodeAddress);
    void addOrUpdateNode(
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
    );

    bool insertLogEvent(const LogInfo log);
    QList<QStringList> getLogEvent(const QString &type, qint64 startDate, qint64 endDate);
    QList<QStringList> getLogEventPaged(const QString &type, qint64 startDate, qint64 endDate, int page);
    QList<QStringList> getAllTestLogs();

    void readNodesForTree();
    void updateRelayMode(uint16_t nodeAddress, bool enabled);
    uint16_t getFatherRealAddress(uint16_t nodeAddress);
    int getCountOfDirectChildren(uint16_t nodeAddress);
    QList<uint16_t> getChildrenRealAddresses(uint16_t nodeAddress);
    QString getNextNodeName(uint16_t doneIts); // Node 1, Node 2, Node 521...

    uint16_t getMasterRealAddress(void);
    void setMasterRealAddress(uint16_t newAntennaAddress);
    QString getNetKey(void);
    void setNetKey(QString netKey);

    void setRecoveryNode(uint8_t subnetAddress, uint8_t nodeSubnetAddress, uint16_t realAddress, uint8_t *nodeUUID);
    void setFatherRealAddress(uint16_t nodeAddress, uint16_t fatherRealAddress);
    void loadFailComCycles(void);
    void updateFailComCycles(uint8_t cycles);

    void changePosition(uint8_t subnetAddress, uint8_t nodeSubnetAddress, uint16_t realAddress);

    bool isExistingNode(uint16_t realAddress);

    ReplaceNode getNodeDataForReplace(uint16_t realAddress);
    void setNodeDataForReplace(ReplaceNode replaceNode, uint16_t realAddress);
    uint16_t getNodeNetAddressForReplace(uint16_t realAddress);

signals:

private:
    QSqlDatabase _database;
};

#endif // DATABASE_H
