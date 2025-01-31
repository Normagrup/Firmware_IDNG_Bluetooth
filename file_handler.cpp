#include <QDebug>
#include <QRegularExpression>

#include "file_handler.h"
#include "global_variables.h"

void setWebServerData()
{
    QFile file(WEB_SERVER_DATA_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { }//qDebug() << "NO SE PUEDE ABRIR WEB SERVER"; }
    else {
        QTextStream fileInput(&file);
        while (!fileInput.atEnd()) {
            QString line = fileInput.readLine();
            QStringList lineParts = line.split(":");

            if (lineParts.size() == 2) {
                QString key = lineParts[0].trimmed();
                QString value = lineParts[1].trimmed();

                if (key == "Network IP") { webServerData.networkIP = value; }
                else if (key == "Network Submask") { webServerData.networkSubmask = value; }
                else if (key == "Gateway Address") { webServerData.gatewayAddress = value; }
                else if (key == "Building Name") { webServerData.buildingName = value; }
                else if (key == "Line Name") { webServerData.lineName = value; }
                else if (key == "Password") { webServerData.password = value; }
                else if (key == "Mantenedor Password") { webServerData.mantenedorPassword = value; }
            }
        }

        QStringList networkIPParts = webServerData.networkIP.split(".");
        for (uint8_t i = 0; i < 4; i++) { networkIP[i] = networkIPParts[i].trimmed(); }

        QStringList networkSubmaskParts = webServerData.networkSubmask.split(".");
        for (uint8_t i = 0; i < 4; i++) { networkSubmask[i] = networkSubmaskParts[i].trimmed(); }

        QStringList gatewayAddressParts = webServerData.gatewayAddress.split(".");
        for (uint8_t i = 0; i < 4; i++) { gatewayAddress[i] = gatewayAddressParts[i].trimmed(); }

        file.close();
    }
}

QString getInterfacesConfig(QString config)
{
    QFile file(INTERFACES_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { qDebug() << "NO SE PUEDE ABRIR EL ARCHIVO"; }

    QTextStream in(&file);
    QString interfaceConfig;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.startsWith(config)) {
            interfaceConfig = line.split(" ").last();
            break;
        }
    }

    file.close();
    return interfaceConfig;
}

void getMacAddress()
{
    QFile file(MAC_ADDRESS_PATH);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) { qDebug() << "NO SE PUEDE ABRIR MAC ADDRESS"; }
    else {
        QTextStream fileInterface(&file);
        QString line = fileInterface.readLine();
        QStringList lineParts = line.split(":");

        for (uint8_t i = 0; i < 6; i++) { macAddress[i] = lineParts[i].trimmed(); }

        file.close();
    }
}

void getAdminPassword(uint8_t* data)
{
    QByteArray passwordByteArray = webServerData.password.toLatin1();
    for (uint8_t i = 0; i < passwordByteArray.size(); i++) { data[i] = passwordByteArray.at(i); }
    for (uint8_t i = passwordByteArray.size(); i < 16; i++) { data[i] = 0; }
}

void getMantenedorPassword(uint8_t* data)
{
    QByteArray mantenedorPasswordByteArray = webServerData.mantenedorPassword.toLatin1();
    for (uint8_t i = 0; i < mantenedorPasswordByteArray.size(); i++) { data[i] = mantenedorPasswordByteArray.at(i); }
    for (uint8_t i = mantenedorPasswordByteArray.size(); i < 16; i++) { data[i] = 0; }
}

void getBuildingName(uint8_t* data)
{
    QByteArray buildingNameByteArray = webServerData.buildingName.toLatin1();
    for (uint8_t i = 0; i < buildingNameByteArray.size(); i++) { data[i] = buildingNameByteArray.at(i); }
    for (uint8_t i = buildingNameByteArray.size(); i < 16; i++) { data[i] = 0; }
}

void getLineName(uint8_t* data)
{
    QByteArray lineNameByteArray = webServerData.lineName.toLatin1();
    for (uint8_t i = 0; i < lineNameByteArray.size(); i++) { data[i] = lineNameByteArray.at(i); }
    for (uint8_t i = lineNameByteArray.size(); i < 16; i++) { data[i] = 0; }
}

void getRtcDate(uint8_t* data)
{
    QProcess process;
    process.start("timedatectl");
    if (!process.waitForFinished()) { }

    QByteArray output = process.readAllStandardOutput();
    if (output.contains("RTC time")) {
        QRegularExpression regex("RTC time: \\w+ (\\d{4}-\\d{2}-\\d{2}) \\d{2}:\\d{2}:\\d{2}");
        QRegularExpressionMatch match = regex.match(output);
        if (match.hasMatch()) {
            QString rtcDate = match.captured(1);
            QStringList rtcDateParts = rtcDate.split("-");

            for (uint8_t i = 0; i < 3; i++) { data[i] = rtcDateParts[i].trimmed().toUInt(); }

            data[3] = data[2] / 7 + (data[2] % 7 > 0 ? 1 : 0);
        }
    }
}

void getRtcSeconds(uint8_t* data)
{
    QProcess process;
    process.start("timedatectl");
    if (!process.waitForFinished()) { }

    QByteArray output = process.readAllStandardOutput();
    if (output.contains("RTC time")) {
        QRegularExpression regex("RTC time: \\w+ \\d{4}-\\d{2}-\\d{2} (\\d{2}):(\\d{2}):(\\d{2})");
        QRegularExpressionMatch match = regex.match(output);
        if (match.hasMatch()) {
            data[2] = match.captured(1).trimmed().toUInt();
            data[1] = match.captured(2).trimmed().toUInt();
            data[0] = match.captured(3).trimmed().toUInt();
        }
    }
}

QString getLocalDate()
{
    QProcess process;
    process.start("timedatectl");
    if (!process.waitForFinished()) { }

    QByteArray output = process.readAllStandardOutput();
    if (output.contains("Local time")) {
        QRegularExpression regex("Local time: \\w+ (\\d{4}-\\d{2}-\\d{2}) \\d{2}:\\d{2}:\\d{2}");
        QRegularExpressionMatch match = regex.match(output);
        if (match.hasMatch()) {
            QString date = match.captured(1);
            return date;
        }
    }
}

QString getLocalTime()
{
    QProcess process;
    process.start("timedatectl");
    if (!process.waitForFinished()) { }

    QByteArray output = process.readAllStandardOutput();
    if (output.contains("Local time")) {
        QRegularExpression regex("Local time: \\w+ \\d{4}-\\d{2}-\\d{2} (\\d{2}:\\d{2}:\\d{2})");
        QRegularExpressionMatch match = regex.match(output);
        if (match.hasMatch()) {
            QString time = match.captured(1);
            return time;
        }
    }
}

QString getLocalDay()
{
    QProcess process;
    process.start("timedatectl");
    if (!process.waitForFinished()) { }

    QByteArray output = process.readAllStandardOutput();
    if (output.contains("Local time")) {
        QRegularExpression regex("Local time:\\s+(\\w+) \\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}");
        QRegularExpressionMatch match = regex.match(output);
        if (match.hasMatch()) {
            QString day = match.captured(1);
            return day;
        }
    }
}

void rebootDevice()
{
    QProcess process;
    process.start("sudo", QStringList() << "reboot");
    process.waitForFinished(-1);
}

void setIPAddressFile(QString ipAddress)
{
    QFile interfacesFile(INTERFACES_PATH);
    if (!interfacesFile.open(QIODevice::ReadWrite | QIODevice::Text)) { qDebug() << "NO SE PUEDE ABRIR INTERFACES"; }
    else {
        QTextStream interfacesInput(&interfacesFile);
        QStringList interfacesLines;
        while (!interfacesInput.atEnd()) {
            QString line = interfacesInput.readLine();
            interfacesLines.append(line);
        }

        interfacesLines[2] = "address " + ipAddress;
        interfacesFile.seek(0);

        QTextStream interfacesOutput(&interfacesFile);
        for (const QString& line : interfacesLines) { interfacesOutput << line << endl; }
        interfacesFile.close();
    }
}

void setSubmaskAddressFile(QString submaskAddress)
{
    QFile interfacesFile(INTERFACES_PATH);
    if (!interfacesFile.open(QIODevice::ReadWrite | QIODevice::Text)) { qDebug() << "NO SE PUEDE ABRIR INTERFACES"; }
    else {
        QTextStream interfacesInput(&interfacesFile);
        QStringList interfacesLines;
        while (!interfacesInput.atEnd()) {
            QString line = interfacesInput.readLine();
            interfacesLines.append(line);
        }

        interfacesLines[3] = "netmask " + submaskAddress;
        interfacesFile.seek(0);

        QTextStream interfacesOutput(&interfacesFile);
        for (const QString& line : interfacesLines) { interfacesOutput << line << endl; }
        interfacesFile.close();
    }
}

void setGatewayAddressFile(QString gatewayAddress)
{
    QFile interfacesFile(INTERFACES_PATH);
    if (!interfacesFile.open(QIODevice::ReadWrite | QIODevice::Text)) { qDebug() << "NO SE PUEDE ABRIR INTERFACES"; }
    else {
        QTextStream interfacesInput(&interfacesFile);
        QStringList interfacesLines;
        while (!interfacesInput.atEnd()) {
            QString line = interfacesInput.readLine();
            interfacesLines.append(line);
        }

        interfacesLines[4] = "gateway " + gatewayAddress;
        interfacesFile.seek(0);

        QTextStream interfacesOutput(&interfacesFile);
        for (const QString& line : interfacesLines) { interfacesOutput << line << endl; }
        interfacesFile.close();
    }
}

void setBuildingNameFile(QString buildingName)
{
    QFile webServerFile(WEB_SERVER_DATA_PATH);
    if (!webServerFile.open(QIODevice::ReadWrite | QIODevice::Text)) { qDebug() << "NO SE PUEDE ABRIR WEB SERVER"; }
    else {
        QTextStream webServerInput(&webServerFile);
        QStringList webServerLines;
        while (!webServerInput.atEnd()) {
            QString line = webServerInput.readLine();
            webServerLines.append(line);
        }

        webServerLines[3] = "Building Name: " + buildingName;
        webServerFile.seek(0);

        QTextStream webServerOutput(&webServerFile);
        for (const QString& line : webServerLines) { webServerOutput << line << endl; }
        webServerFile.close();
    }
}

void setLineNameFile(QString lineName)
{
    QFile webServerFile(WEB_SERVER_DATA_PATH);
    if (!webServerFile.open(QIODevice::ReadWrite | QIODevice::Text)) { qDebug() << "NO SE PUEDE ABRIR WEB SERVER"; }
    else {
        QTextStream webServerInput(&webServerFile);
        QStringList webServerLines;
        while (!webServerInput.atEnd()) {
            QString line = webServerInput.readLine();
            webServerLines.append(line);
        }

        webServerLines[4] = "Line Name: " + lineName;
        webServerFile.seek(0);

        QTextStream webServerOutput(&webServerFile);
        for (const QString& line : webServerLines) { webServerOutput << line << endl; }
        webServerFile.close();
    }
}

void setRtcDate(QString date)
{
    QProcess process;
    QStringList command;
    command << "timedatectl" << "set-time" << date;
    process.start("sudo", command);
    process.waitForFinished(-1);
}

void setRtcTime(QString time)
{
    QProcess process;
    QStringList command;
    command << "timedatectl" << "set-time" << time;
    process.start("sudo", command);
    process.waitForFinished(-1);
}

void setLocalDateTime(QStringList dateTimeParts)
{
    QString date = dateTimeParts[0];
    QString time = dateTimeParts[1] + ":00";
    QString dateTime = date + " " + time;

    QProcess process;
    QStringList command;
    command << "timedatectl" << "set-time" << dateTime;
    process.start("sudo", command);
    process.waitForFinished(-1);
}

void setAdminPasswordFile(QString adminPassword)
{
    QFile webServerFile(WEB_SERVER_DATA_PATH);
    if (!webServerFile.open(QIODevice::ReadWrite | QIODevice::Text)) { qDebug() << "NO SE PUEDE ABRIR WEB SERVER"; }
    else {
        QTextStream webServerInput(&webServerFile);
        QStringList webServerLines;
        while (!webServerInput.atEnd()) {
            QString line = webServerInput.readLine();
            webServerLines.append(line);
        }

        webServerLines[5] = "Password: " + adminPassword;
        webServerFile.seek(0);

        QTextStream webServerOutput(&webServerFile);
        for (const QString& line : webServerLines) { webServerOutput << line << endl; }
        webServerFile.close();
    }
}

void setMantenedorPasswordFile(QString mantenedorPassword)
{
    QFile webServerFile(WEB_SERVER_DATA_PATH);
    if (!webServerFile.open(QIODevice::ReadWrite | QIODevice::Text)) { qDebug() << "NO SE PUEDE ABRIR WEB SERVER"; }
    else {
        QTextStream webServerInput(&webServerFile);
        QStringList webServerLines;
        while (!webServerInput.atEnd()) {
            QString line = webServerInput.readLine();
            webServerLines.append(line);
        }

        webServerLines[6] = "Mantenedor Password: " + mantenedorPassword;
        webServerFile.seek(0);

        QTextStream webServerOutput(&webServerFile);
        for (const QString& line : webServerLines) { webServerOutput << line << endl; }
        webServerFile.close();
    }
}
