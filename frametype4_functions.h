#ifndef FRAMETYPE4_FUNCTIONS_H
#define FRAMETYPE4_FUNCTIONS_H

#include <QObject>

void setIPAddress(QByteArray data);
void setSubmaskAddress(QByteArray data);
void setGatewayAddress(QByteArray data);
void setBuildingName(QByteArray data);
void setLineName(QByteArray data);
void setRtcDateDay(QByteArray data);
void setRtcDateTime(QByteArray data);
void setAdminPassword(QByteArray data);
void setMantenedorPassword(QByteArray data);

#endif // FRAMETYPE4_FUNCTIONS_H
