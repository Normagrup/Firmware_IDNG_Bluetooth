#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

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
    QString getInterfaceParameters(void);
    void setInterfaceParameters(QStringList interfaceParameters);
    void loadNodesFromDatabase(void);
    void loadTestsFromDatabase(void);

    void setNewNode(uint8_t subnetAddress, uint8_t nodeSubnetAddress, uint16_t realAddress, uint8_t* nodeUUID);
    void setGroup(uint16_t realAddress, uint16_t groupAddress);
    void setNodeFeatures(uint16_t nodeAddress, uint8_t deviceType, uint8_t ratedDuration, uint8_t emergencyFeatures, uint8_t physicalMinLvl);
    void setNodeRegister(QString nodeRegister, uint16_t nodeAddress, uint8_t value);

    void deleteNode(uint16_t nodeAddress);
    void delGroup(uint16_t realAddress, uint16_t groupAddress);

    void setTestEnable(QString groupAddress, bool isFunctionalEnable, bool isDurationEnable);
    void setFunctionalTest(QString groupAddress, QString functionalDays, QString functionalTime);
    void setDurationTest(QString groupAddress, QString durationPeriodicity, QString durationDate, QString durationTime);

    QList<uint16_t> getConfiguredNodes(void);

signals:

private:
    QSqlDatabase _database;
};

#endif // DATABASE_H
