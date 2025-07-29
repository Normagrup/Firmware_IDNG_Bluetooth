#include "frametype4_functions.h"
#include "file_handler.h"
#include <QDebug>
#include "aux_functions.h"
#include "time_functions.h"
#include "process_webserver_data.h"

void setIPAddress(QByteArray data, Database* _database)
{
    if ((unsigned char)data[9] == 0x03) {
        QString ipAddress = QString::number((unsigned char)data[10]) + "." + QString::number((unsigned char)data[11]) + "." +
                            QString::number((unsigned char)data[12]) + "." + QString::number((unsigned char)data[13]);
        setIPAddressFile(ipAddress);
        _database->setGeneralData("IP", ipAddress);
        //_database->setIPAddress(ipAddress);
    }
}

void setSubmaskAddress(QByteArray data, Database* _database)
{
    if ((unsigned char)data[9] == 0x03) {
        QString submaskAddress = QString::number((unsigned char)data[10]) + "." + QString::number((unsigned char)data[11]) + "." +
                                 QString::number((unsigned char)data[12]) + "." + QString::number((unsigned char)data[13]);
        setSubmaskAddressFile(submaskAddress);
          _database->setGeneralData("Submask", submaskAddress);
         //_database->setSubmask(submaskAddress);
    }
}

void setGatewayAddress(QByteArray data, Database* _database)
{
    if ((unsigned char)data[9] == 0x03) {
        QString gatewayAddress = QString::number((unsigned char)data[10]) + "." + QString::number((unsigned char)data[11]) + "." +
                                 QString::number((unsigned char)data[12]) + "." + QString::number((unsigned char)data[13]);
        setGatewayAddressFile(gatewayAddress);
         _database->setGeneralData("Gateway", gatewayAddress);
        //_database->setGateway(gatewayAddress);
    }
}

void setBuildingName(QByteArray data, Database* _database)
{
    if ((unsigned char)data[9] == 0x0F && data.size() >= 26) {
        QString buildingName;
        for (uint8_t i = 0; i < 16; i++) {
            if ((i + 10) >= data.size()) break; //safety check
            char asciiChar = static_cast<char>(data[i + 10]);
            if (asciiChar != '\0') { buildingName.append(asciiChar); }
        }
        _database->setGeneralData("BuildingName", buildingName);
        //_database->setBuildingName(buildingName);
    }
}

void setLineName(QByteArray data, Database* _database)
{
    if ((unsigned char)data[9] == 0x0F) {
        QString lineName;
        for (uint8_t i = 0; i < 16; i++) {
            char asciiChar = static_cast<char>(data[i + 10]);
            if (asciiChar != '\0') { lineName.append(asciiChar); }
        }
        _database->setGeneralData("LineName", lineName);
        //_database->setLineName(lineName);
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

void sendGroupNamesToEth(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, Database* _database, UdpSocket* _udpSocket)
{
    QList<QPair<QString, QString>> groupList = _database->getGroups();

    if(groupList.isEmpty()){
        sendGroupNamesFrame(rcvAddress, commandHigh, commandLow, 0, 0, _udpSocket);
    }
    for (int i = 0; i < groupList.size(); ++i) {
        const QString& name = groupList[i].second;
        sendGroupNamesFrame(rcvAddress, commandHigh, commandLow, static_cast<uint8_t>(i), name, _udpSocket);
        delay(ETH_SEND_TIME_MS);
    }
}

QList<GroupBitmap> collectGroupBitmaps(Database* _database)
{
    QList<GroupBitmap> result;

    uint16_t groupAddress;

    int totalGroups = 4 +  _database->getGroups().size();

    for (int groupId = 0; groupId < totalGroups; ++groupId)
    {
        if (groupId < 4) {
            groupAddress = 0xC000 + groupId;
        } else {
            groupAddress = 0xC010 + (groupId - 4);
        }

        for (uint8_t subnet = 0; subnet < MAX_SUBNET; ++subnet)
        {
            if (!polling.isSubnetConfigured(subnet))
                continue;
            QByteArray bitmap(8, 0x00);

            for (int i = 0; i < MAX_NODES_SUBNET; ++i)
            {
                if (meshDevice[subnet][i].getIsConfigured() &&
                     _database->deviceIsInGroup(meshDevice[subnet][i].getRealAddress(), groupAddress))
                {
                    int byteIndex = i / 8;
                    int bitIndex = i % 8;

                    bitmap[byteIndex] = static_cast<uchar>(bitmap[byteIndex]) | (1 << bitIndex);
                }
            }

            if (std::any_of(bitmap.begin(), bitmap.end(), [](char b){ return b != 0x00; }))
            {
                result.append({(uint8_t)groupId, subnet, bitmap});
            }
        }
    }
    return result;
}

void sendGroupDataFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket, const GroupBitmap& gb)
{
    QByteArray frame;
    uchar crc = 0;

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(10);  // 1 groupId + 1 subnet + 8 bytes

    frame.append(gb.groupId);
    frame.append(gb.subnetId);
    frame.append(gb.bitmap);

    for (int i = 3; i < frame.size(); ++i) crc += frame[i];
    frame.append(crc);

    QHostAddress dst;
    dst.setAddress(rcvAddress);
    _udpSocket->sendData(dst, frame);
}

void sendGroupDataToEth(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, Database* _database, UdpSocket *_udpSocket)
{
    QList<GroupBitmap> list = collectGroupBitmaps(_database);

    for (const GroupBitmap& gb : list)
    {
        sendGroupDataFrame(rcvAddress, commandHigh, commandLow, _udpSocket, gb);
        delay(ETH_SEND_TIME_MS);
    }
}

void saveGroupFromEth(QByteArray data, Database* _database)
{
    uint8_t groupId = static_cast<int>(data[10]);
    uint8_t nameLength = static_cast<int>(data[9]);
    QByteArray nameBytes = data.mid(11, nameLength);
    QString newName = QString::fromUtf8(nameBytes).trimmed();

    uint16_t groupValue = getMaskedGroupId(groupId);
    QString groupAddress =  QString("C%1").arg(groupValue & 0x0FFF, 3, 16, QLatin1Char('0')).toUpper();
    _database->editGroup(groupAddress, newName);
}

void updateGroupsDataFromEth(QByteArray data,  Database* _database, UartPort* _uartPort)
{
    writeGroupBitmap gb;
    gb.groupId = data[10];
    gb.subnetId = data[11];
    gb.bitmap = data.mid(12, 8);

    pendingGroupBitmaps.append(gb);
    processGroupBitmap(gb, _database, _uartPort);  //First try
    int durMS = numDevicesToUpdate * 300;
    QTimer::singleShot(durMS + 200, [=]() {
        if(!pendingGroupUpdatesEth.isEmpty())
            processGroupBitmap(gb, _database, _uartPort);  //Retry after delay
    });
}

void sendTestDataFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket, QByteArray testData)
{
    QByteArray frame;
    uchar crc = 0;

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x10);

    frame.append(testData);

    for (int i = 3; i < frame.size(); ++i) crc += frame[i];
    frame.append(crc);

    QHostAddress dst;
    dst.setAddress(rcvAddress);

    _udpSocket->sendData(dst, frame);
}

void sendTestDataToEth(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket, Database* _database, QByteArray data)
{
    uint8_t groupId = static_cast<int>(data[10]);
    QString groupAddress;
    if(groupId == 255){
        groupAddress = "FFFF";
    } else {
        uint16_t groupValue = getMaskedGroupId(groupId);
        groupAddress =  QString("C%1").arg(groupValue & 0x0FFF, 3, 16, QLatin1Char('0')).toUpper();
    }

    if(_database->groupExistsInTestTable(groupAddress)){
        QString testStr = _database->getTests(groupAddress);
        QStringList parts = testStr.split("#");

        int functionalEnable = parts[0].toInt();
        int durationEnable   = parts[1].toInt();
        QStringList daysList = parts[2].split(" ");
        QString functionalTime = parts[3];
        QString durPeriodicity = parts[4];
        QString durationDate = parts[5];
        QString durationTime = parts[6];

        uchar fuEnable = functionalEnable;
        uchar durEnable = durationEnable;
        uchar weekday = 0;
        for (const QString& day : daysList) {
            int d = mapDayToNumber(day.trimmed());
            if (d >= 1 && d <= 7)
                weekday |= (1 << (d - 1));
        }

        uchar fuHour = functionalTime.left(2).toInt();
        uchar fuMin  = functionalTime.right(2).toInt();
        uchar dtYear = durationDate.mid(2, 2).toUInt();
        uchar dtDay = durationDate.mid(8,2).toInt();
        uchar dtMonth = durationDate.mid(5,2).toInt();
        uchar dtHour = durationTime.left(2).toInt();
        uchar dtMin  = durationTime.right(2).toInt();
        uchar periodicity = durPeriodicity.toInt();

        QByteArray testData;
        testData.append(fuEnable);
        testData.append(durEnable);
        testData.append(weekday);
        testData.append(fuHour);
        testData.append(fuMin);
        testData.append(dtYear);
        testData.append(dtDay);
        testData.append(dtMonth);
        testData.append(dtHour);
        testData.append(dtMin);
        testData.append(periodicity);

        sendTestDataFrame(rcvAddress, commandHigh, commandLow, _udpSocket, testData);
    }

}

void setTestDataFromEth(QByteArray data, Database* _database)
{
    uint8_t groupId       = static_cast<int>(data[10]);
    uchar fuEnable        = data[11];
    uchar durEnable       = data[12];
    uchar weekdayBitmask  = data[13];
    uchar fuHour = QString::number(data[14], 16).toUInt();
    uchar fuMin  = QString::number(data[15], 16).toUInt();
    uchar year   = QString::number(data[16], 16).toUInt();
    uchar dtMonth = QString::number(data[17], 16).toUInt();
    uchar dtDay   = QString::number(data[18], 16).toUInt();
    uchar dtHour  = QString::number(data[19], 16).toUInt();
    uchar dtMin   = QString::number(data[20], 16).toUInt();
    uchar periodicity = QString::number(data[21], 16).toUInt();
    QString groupAddress;

    if(groupId == 255){
        groupAddress = "FFFF";
    } else {
        uint16_t groupValue = getMaskedGroupId(groupId);
        groupAddress =  QString("C%1").arg(groupValue & 0x0FFF, 3, 16, QLatin1Char('0')).toUpper();
    }

    QStringList selectedDays;
    for (int i = 0; i < 7; ++i) {
        if ((weekdayBitmask >> i) & 1) {
            selectedDays << mapWeekdayToName(i + 1);
        }
    }
    QString functionalDays = selectedDays.join(" ") + " ";
    QString functionalTime = QString("%1:%2")
                                 .arg(fuHour, 2, 10, QChar('0'))
                                 .arg(fuMin, 2, 10, QChar('0'));
    QString durationTime = QString("%1:%2")
                               .arg(dtHour, 2, 10, QChar('0'))
                               .arg(dtMin, 2, 10, QChar('0'));

    int nowYear = 2000 + year;
    QString durationDate = QString("%1-%2-%3")
                               .arg(nowYear)
                               .arg(dtMonth, 2, 10, QChar('0'))
                               .arg(dtDay, 2, 10, QChar('0'));

    QStringList wsParts;
    wsParts << groupAddress
            << QString::number(fuEnable)
            << functionalDays
            << functionalTime
            << QString::number(durEnable)
            << QString::number(periodicity)
            << durationDate
            << durationTime;

    setTests(wsParts, _database);
}

void sendLogDataToEth(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket, Database *_database, QByteArray data)
{
    uint16_t pos = (static_cast<uint8_t>(data[10]) << 8) | static_cast<uint8_t>(data[11]); //pos (2 bytes: high, low)

    QList<QStringList> logs = _database->getLastNLogEvents(pos);
    int totalLogs = logs.size();
    uint16_t Id;
    bool group = false;

    if(pos > totalLogs)
        pos = totalLogs;

    for (int i = 0; i < pos; i++)
    {
        const QStringList &log = logs[i];
        if (log.size() < 6) continue;
        if(log[2].toUInt() >= 49152){
            group = true;
            Id = getGroupIdFromMasked(log[2].toUInt());
        } else if (log[2] == "-1"){
            group = true;
            Id = -1;
        } else {
            group = false;
            Id = log[2].toUInt();
        }

        QByteArray frame;
        uchar crc = 0;

        frame.append(FRAME_HEADER_0);
        frame.append(FRAME_HEADER_1);
        frame.append(FRAME_HEADER_2);
        frame.append(FRAME_TYPE_83);
        frame.append(commandHigh);
        frame.append(commandLow);
        frame.append(0x0F);

        QByteArray payload(16, 0xFF);

        payload[0] = 0x01;
        payload[1] = 0xFF;
        payload[2] = Id;  // BtAddress as shortAddress

        // Timestamp
        QDateTime dt = QDateTime::fromString(log[4], "yyyy-MM-dd HH:mm:ss");
        payload[3] = dt.date().day();
        payload[4] = dt.date().month();
        payload[5] = dt.date().year() - 2000;
        payload[6] = dt.time().hour();
        payload[7] = dt.time().minute();

        payload[8] = log[5].toUInt();  // Event code

        // Serial
        QStringList serial = log[1].split(".");
        if (serial.size() == 4) {
            payload[9]  = serial[0].toUInt(nullptr, 16);
            payload[10] = serial[1].toUInt(nullptr, 16);
            payload[11] = serial[2].toUInt(nullptr, 16);
            payload[12] = serial[3].toUInt(nullptr, 16);
        }

        // Device name
        if (!group && log[0].startsWith("A")) {
            bool ok;
            int globalPos = log[0].mid(1).toInt(&ok);
            if (ok) {
                int subnet = (globalPos - 1) / 64;
                int id = (globalPos - 1) % 64;

                payload[13] = static_cast<uchar>(subnet);
                payload[14] = static_cast<uchar>(id);
            }
        }

        frame.append(payload);

        for (int j = 3; j < frame.size(); ++j)
            crc += frame[j];
        frame.append(crc);

        QHostAddress dst;
        dst.setAddress(rcvAddress);
        _udpSocket->sendData(dst, frame);

        delay(ETH_SEND_TIME_MS);
    }
}

void processGroupBitmap(const writeGroupBitmap &gb, Database* _database, UartPort* _uartPort)
{
    uint16_t groupAddress = getMaskedGroupId(gb.groupId);
    uint8_t subnet = gb.subnetId;
    QByteArray bitmap = gb.bitmap;

    for (int node = 0; node < MAX_NODES_SUBNET; node++)
    {
        Device &dev = meshDevice[subnet][node];

        if (!dev.getIsConfigured())
            continue;

        int byteIndex = node / 8;
        int bitIndex  = node % 8;

        bool toAddToGroup = (bitmap[byteIndex] >> bitIndex) & 0x01;
        uint16_t realAddress = dev.getRealAddress();
        bool isInGroup = _database->deviceIsInGroup(realAddress, groupAddress);

        if (toAddToGroup && !isInGroup) {
            numDevicesToUpdate++;
            uint16_t address[3] = { realAddress, groupAddress, 0x0000 };
            QString key = QString("%1:%2").arg(realAddress).arg(groupAddress);
            pendingGroupUpdatesEth.insert(key);
            groupActionTypeMap[key] = Add;
            sendUartAddGroupManual(_uartPort, address);
            delay(300);
        }
        else if (!toAddToGroup && isInGroup) {
            numDevicesToUpdate++;
            uint16_t address[3] = { realAddress, groupAddress, 0x0000 };
            QString key = QString("%1:%2").arg(realAddress).arg(groupAddress);
            pendingGroupUpdatesEth.insert(key);
            groupActionTypeMap[key] = Remove;
            sendUartDelGroup(_uartPort, address, _database);
            delay(300);
        }
    }
}


void swapDeviceFromEth(QByteArray data, Database *_database)
{
    uint16_t pos1 = static_cast<uint8_t>(data[10]) | (static_cast<uint8_t>(data[11]) << 8);
    uint16_t pos2 = static_cast<uint8_t>(data[12]) | (static_cast<uint8_t>(data[13]) << 8);

    changePositions(_database, pos1, pos2);
}
