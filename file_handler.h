#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <QFile>
#include <QTextStream>
#include <QProcess>

#define WEB_SERVER_DATA_PATH        "/normagrup/www/webserver.txt"
#define MAC_ADDRESS_PATH            "/sys/class/net/eth0/address"
#define INTERFACES_PATH             "/etc/network/interfaces"

void setWebServerData(void);

QString getInterfacesConfig(QString config);
void getMacAddress(void);
void getAdminPassword(uint8_t* data);
void getMantenedorPassword(uint8_t* data);
void getBuildingName(uint8_t* data);
void getLineName(uint8_t* data);
void getRtcDate(uint8_t* data);
void getRtcSeconds(uint8_t* data);
QString getLocalDate(void);
QString getLocalTime(void);
QString getLocalDay(void);

void rebootDevice(void);

void setIPAddressFile(QString ipAddress);
void setSubmaskAddressFile(QString submaskAddress);
void setGatewayAddressFile(QString gatewayAddress);
void setBuildingNameFile(QString buildingName);
void setLineNameFile(QString lineName);
void setRtcDate(QString date);
void setRtcTime(QString time);
void setLocalDateTime(QStringList dateTime);
void setAdminPasswordFile(QString adminPassword);
void setMantenedorPasswordFile(QString mantenedorPassword);

#endif // FILE_HANDLER_H
