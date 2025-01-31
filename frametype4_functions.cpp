#include "frametype4_functions.h"
#include "file_handler.h"
#include <QDebug>

void setIPAddress(QByteArray data)
{
    if ((unsigned char)data[9] == 0x03) {
        QString ipAddress = QString::number((unsigned char)data[10]) + "." + QString::number((unsigned char)data[11]) + "." +
                            QString::number((unsigned char)data[12]) + "." + QString::number((unsigned char)data[13]);
        setIPAddressFile(ipAddress);
    }
}

void setSubmaskAddress(QByteArray data)
{
    if ((unsigned char)data[9] == 0x03) {
        QString submaskAddress = QString::number((unsigned char)data[10]) + "." + QString::number((unsigned char)data[11]) + "." +
                                 QString::number((unsigned char)data[12]) + "." + QString::number((unsigned char)data[13]);
        setSubmaskAddressFile(submaskAddress);
    }
}

void setGatewayAddress(QByteArray data)
{
    if ((unsigned char)data[9] == 0x03) {
        QString gatewayAddress = QString::number((unsigned char)data[10]) + "." + QString::number((unsigned char)data[11]) + "." +
                                 QString::number((unsigned char)data[12]) + "." + QString::number((unsigned char)data[13]);
        setGatewayAddressFile(gatewayAddress);
    }
}

void setBuildingName(QByteArray data)
{
    if ((unsigned char)data[9] == 0x0F) {
        QString buildingName;
        for (uint8_t i = 0; i < 16; i++) {
            char asciiChar = static_cast<char>(data[i + 10]);
            if (asciiChar != '\0') { buildingName.append(asciiChar); }
        }
        setBuildingNameFile(buildingName);
    }
}

void setLineName(QByteArray data)
{
    if ((unsigned char)data[9] == 0x0F) {
        QString lineName;
        for (uint8_t i = 0; i < 16; i++) {
            char asciiChar = static_cast<char>(data[i + 10]);
            if (asciiChar != '\0') { lineName.append(asciiChar); }
        }
        setLineNameFile(lineName);
    }
}

void setRtcDateDay(QByteArray data)
{
    if ((unsigned char)data[9] == 0x03) {
        uint8_t day, month, year;
        day = (unsigned char)data[11];
        month = (unsigned char)data[12];
        year = (unsigned char)data[13];

        QString date = QString::number(year, 16) + "-" + QString::number(month, 16) + "-" + QString::number(day, 16);
        setRtcDate(date);
    }
}

void setRtcDateTime(QByteArray data)
{
    if ((unsigned char)data[9] == 0x02) {
        uint8_t seconds, minutes, hours;
        hours = (unsigned char)data[10];
        minutes = (unsigned char)data[11];
        seconds = (unsigned char)data[12];

        QString time = QString::number(hours, 16) + ":" + QString::number(minutes, 16) + ":" + QString::number(seconds, 16);
        setRtcTime(time);
    }
}

void setAdminPassword(QByteArray data)
{
    if ((unsigned char)data[9] == 0x0F) {
        QString adminPassword;
        for (uint8_t i = 0; i < 16; i++) {
            char asciiChar = static_cast<char>(data[i + 10]);
            if (asciiChar != '\0') { adminPassword.append(asciiChar); }
        }
        setAdminPasswordFile(adminPassword);
    }
}

void setMantenedorPassword(QByteArray data)
{
    if ((unsigned char)data[9] == 0x0F) {
        QString mantenedorPassword;
        for (uint8_t i = 0; i < 16; i++) {
            char asciiChar = static_cast<char>(data[i + 10]);
            if (asciiChar != '\0') { mantenedorPassword.append(asciiChar); }
        }
        setMantenedorPasswordFile(mantenedorPassword);
    }
}
