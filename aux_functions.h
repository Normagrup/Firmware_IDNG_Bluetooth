#ifndef AUX_FUNCTIONS_H
#define AUX_FUNCTIONS_H

#include <QObject>
#include "global_variables.h"
#include "Database.h"

#define SUBNET_GROUPS       0x50

uint16_t getTargetAddress(uint8_t subnet, uint8_t daliAddress);
void QStringToUint8Array(const QString &hexString, uint8_t array[16]);
ScannedUUID compareDeviceUUID(QString receivedUUID);
uint16_t getNodeNetAddress(QString receivedData);
uint16_t* getGroupAddress(QString receivedData);
uint16_t getOneGroupAddress(QString receivedData);
uint16_t* getActualLvl(QString receivedData);
void setFirstAddressAvailable(uint16_t nodeAddress, uint8_t* nodeUUID, Database* database, uint8_t* netAddress);
void convertUuidStringToByteArray(QString uuidString, uint8_t* UUID);
uint8_t convertGroupSubStringToArray(QString groupSubString, uint16_t* groupSubArray);
void setIPConfigInfo(QStringList webServerParts, Database* database);
void insertLogEvent(Database* database, int devId, QString serialNum, QString devName, QString devIP, QDateTime dateTime, int eventCode, QString eventType);
void logTestRequest(Database* db, uint16_t targetAddr, bool isGroup, const QString& testType);
void addTestToChecklist(uint16_t realAddr, const QString& testType, const QDateTime& baseTime);
void insertDevToLog(uint16_t nodeAddress, Database *db, int eventCode, QString eventType);
void insertComsErrorToLog(const QByteArray& uuidArray, Database* db, int eventCode);
AntennaInfo getAntennaInfo(Database* db);
void removeLogTestFromCheckList(uint16_t nodeAddress);

int getUUIDIndexOfScanned(QString UUID);
QString getUUIDAsString(uint8_t UUID[16]);

void setTests(QStringList webServerParts, Database* database);

#endif // AUX_FUNCTIONS_H
