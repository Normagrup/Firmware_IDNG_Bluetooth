#include "frametype4_functions.h"
#include "file_handler.h"
#include <QDebug>
#include "aux_functions.h"
#include "time_functions.h"

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

void sendGroupNamesFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, uint8_t groupId, const QString& groupName, UdpSocket *_udpSocket)
{
    QByteArray frame;
    uchar crc = 0;

    QByteArray nameBytes = groupName.toUtf8();
    uint8_t nameLength = nameBytes.size();

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(1+nameLength);
    frame.append(groupId);
    frame.append(nameLength);
    frame.append(nameBytes.left(nameLength));

    for (int i = 3; i < frame.size(); ++i) crc += frame[i];
    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendGroupNamesToNormalink(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    Database* db;

    QList<QPair<QString, QString>> groupList = db->getGroups();

    for (int i = 0; i < groupList.size(); ++i) {
        const QString& name = groupList[i].second;
        sendGroupNamesFrame(rcvAddress, commandHigh, commandLow, static_cast<uint8_t>(i), name, _udpSocket);
        delay(WEBSERVER_SEND_TIME_MS);
    }
}

