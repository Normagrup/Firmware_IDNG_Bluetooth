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
    uint8_t groupId = data[10];
    uint8_t subnet = data[11];
    QByteArray bitmap = data.mid(12, 8);

    uint16_t groupAddress = getMaskedGroupId(groupId);

    for (int node = 0; node < MAX_NODES_SUBNET; ++node)
    {
        Device &dev = meshDevice[subnet][node];

        if (!dev.getIsConfigured())
            continue;

        int byteIndex = node / 8;
        int bitIndex  = node % 8;

        bool shouldBeInGroup = (bitmap[byteIndex] >> bitIndex) & 0x01;
        uint16_t realAddress = dev.getRealAddress();
        bool isInGroup = _database->deviceIsInGroup(realAddress, groupAddress);

        if (shouldBeInGroup && !isInGroup) {
            //_database->setGroup(realAddress, groupAddress);
            uint16_t address[3] = { realAddress, groupAddress, 0x0000 };
            sendUartAddGroupManual(_uartPort, address);
            delay(SLEEP_DALI_TIME_MS);
        }
        else if (!shouldBeInGroup && isInGroup) {
            uint16_t address[3] = { realAddress, groupAddress, 0x0000 };
            sendUartDelGroup(_uartPort, address, _database);
            delay(SLEEP_DALI_TIME_MS);
        }
    }
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
    frame.append(0x07);

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
    uint16_t groupValue = getMaskedGroupId(groupId);
    QString groupAddress =  QString("C%1").arg(groupValue & 0x0FFF, 3, 16, QLatin1Char('0')).toUpper();

    if(_database->groupExistsInTestTable(groupAddress)){
        QString testStr = _database->getTests(groupAddress);
        QStringList parts = testStr.split("#");

        int functionalEnable = parts[0].toInt();
        int durationEnable   = parts[1].toInt();
        QStringList daysList = parts[2].split(" ");
        QString functionalTime = parts[3];
        QString durationDate = parts[5];
        QString durationTime = parts[6];

        uchar enableFlag = (functionalEnable || durationEnable) ? 0x01 : 0x00;
        uchar weekday = mapDayToNumber(daysList[0]);  // From normalink can choose just one
        uchar fuHour = functionalTime.left(2).toInt();
        uchar fuMin  = functionalTime.right(2).toInt();
        uchar dtDay = durationDate.mid(8,2).toInt();
        uchar dtMonth = durationDate.mid(5,2).toInt();
        uchar dtHour = durationTime.left(2).toInt();
        uchar dtMin  = durationTime.right(2).toInt();

        QByteArray testData;
        testData.append(enableFlag);
        testData.append(weekday);
        testData.append(fuHour);
        testData.append(fuMin);
        testData.append(dtDay);
        testData.append(dtMonth);
        testData.append(dtHour);
        testData.append(dtMin);

        sendTestDataFrame(rcvAddress, commandHigh, commandLow, _udpSocket, testData);
    }

}

void setTestDataFromEth(QByteArray data, Database* _database)
{
    uint8_t groupId        = static_cast<int>(data[10]);
    uchar enabled         =data[11];
    uchar weekday         =data[12];  
    uchar fuHour = QString::number(data[13], 16).toUInt();
    uchar fuMin  = QString::number(data[14], 16).toUInt();
    uchar dtMonth = QString::number(data[15], 16).toUInt();
    uchar dtDay   = QString::number(data[16], 16).toUInt();
    uchar dtHour  = QString::number(data[17], 16).toUInt();
    uchar dtMin   = QString::number(data[18], 16).toUInt();

    uint16_t groupValue = getMaskedGroupId(groupId);
    QString groupAddress =  QString("C%1").arg(groupValue & 0x0FFF, 3, 16, QLatin1Char('0')).toUpper();
    QString functionalDays = mapWeekdayToName(weekday) + " ";
    QString functionalTime = QString("%1:%2")
                                 .arg(fuHour, 2, 10, QChar('0'))
                                 .arg(fuMin, 2, 10, QChar('0'));
    QString durationTime = QString("%1:%2")
                               .arg(dtHour, 2, 10, QChar('0'))
                               .arg(dtMin, 2, 10, QChar('0'));

    QDate now = QDate::currentDate();  // Get current year
    QString durationDate = QString("%1-%2-%3")
                               .arg(now.year())
                               .arg(dtMonth, 2, 10, QChar('0'))
                               .arg(dtDay, 2, 10, QChar('0'));

    QStringList wsParts;
    wsParts << groupAddress
            << QString::number(enabled)
            << functionalDays
            << functionalTime
            << QString::number(enabled)         // use same for duration enable
            << "1"                              // default periodicity
            << durationDate
            << durationTime;

    setTests(wsParts, _database);
}
