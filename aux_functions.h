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
uint16_t* getActualLvl(QString receivedData);
void setFirstAddressAvailable(uint16_t nodeAddress, uint8_t* nodeUUID, Database* database, uint8_t* netAddress);
void convertUuidStringToByteArray(QString uuid, uint8_t* serialNumber);
uint8_t convertGroupSubStringToArray(QString groupSubString, uint16_t* groupSubArray);
void setIPConfigInfo(QStringList webServerParts, Database* database);

void disableAllTest(QStringList webServerParts, Database* database);
void setFunctionalTest(QStringList webServerParts, Database* database);
void setDurationTest(QStringList webServerParts, Database* database);
void setAllTest(QStringList webServerParts, Database* database);

#endif // AUX_FUNCTIONS_H
