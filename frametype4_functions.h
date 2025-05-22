#ifndef FRAMETYPE4_FUNCTIONS_H
#define FRAMETYPE4_FUNCTIONS_H

#include <QObject>
#include "UdpSocket.h"
#include "eth_frames.h"

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
void sendGroupNamesToNormalink(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);

#endif // FRAMETYPE4_FUNCTIONS_H
