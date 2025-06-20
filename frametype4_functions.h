#ifndef FRAMETYPE4_FUNCTIONS_H
#define FRAMETYPE4_FUNCTIONS_H

#include <QObject>
#include "UdpSocket.h"
#include "eth_frames.h"
#include "Database.h"
#include "Pollings.h"

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
void sendGroupDataFrame(const QString& rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket, const GroupBitmap& gb);
QList<GroupBitmap> collectGroupBitmaps();
void sendGroupDataToEth(const QString& rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendGroupNamesToEth(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void SaveGroupFromEth(QByteArray data);

#endif // FRAMETYPE4_FUNCTIONS_H
