#ifndef FRAMETYPE4_FUNCTIONS_H
#define FRAMETYPE4_FUNCTIONS_H

#include <QObject>
#include "UdpSocket.h"
#include "eth_frames.h"
#include "Database.h"
#include "Pollings.h"
#include "process_uart_data.h"


void setIPAddress(QByteArray data);
void setSubmaskAddress(QByteArray data);
void setGatewayAddress(QByteArray data);
void setBuildingName(QByteArray data);
void setLineName(QByteArray data);
void setRtcDateDay(QByteArray data);
void setRtcDateTime(QByteArray data);
void setAdminPassword(QByteArray data);
void setMantenedorPassword(QByteArray data);
void sendGroupNamesFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, uint8_t groupId, const QString& groupName, UdpSocket* _udpSocket);
void sendGroupDataFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket, const GroupBitmap& gb);
QList<GroupBitmap> collectGroupBitmaps( Database* _database);
void sendGroupDataToEth(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, Database* database, UdpSocket* _udpSocket);
void sendGroupNamesToEth(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, Database* _database, UdpSocket* _udpSocket);
void saveGroupFromEth(QByteArray data,  Database* _database);
void processGroupBitmap(const writeGroupBitmap& gb, Database* db, UartPort* uartPort);
void updateGroupsDataFromEth(QByteArray data, Database* _database, UartPort* _uartPort);

void sendTestDataFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket, QByteArray data);
void sendTestDataToEth(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket, Database* _database, QByteArray data);
void setTestDataFromEth(QByteArray data, Database* _database);
void sendLogDataToEth(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket, Database* _database, QByteArray data);

#endif // FRAMETYPE4_FUNCTIONS_H
