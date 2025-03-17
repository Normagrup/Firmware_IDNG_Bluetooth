#include <QDebug>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDate>


#include "file_handler.h"
#include "global_variables.h"

void setWebServerData(Database* database)
{
    QStringList generalData = database->getInterfaceParameters();

    webServerData.networkIP = generalData[0];
    webServerData.networkSubmask = generalData[1];
    webServerData.gatewayAddress = generalData[2];
    webServerData.buildingName = generalData[3];
    webServerData.lineName = generalData[4];

    QStringList loginData = database->getLoginParameters();

    for(QString userPass : loginData) {
        QString user = userPass.split(" ")[0];
        QString pass = userPass.split(" ")[1];

        if(user == "admin")
            webServerData.password = pass;
        else if(user == "fabrica")
            webServerData.mantenedorPassword = pass;
    }

    QStringList networkIPParts = webServerData.networkIP.split(".");
    for (uint8_t i = 0; i < 4; i++) { networkIP[i] = networkIPParts[i].trimmed(); }

    QStringList networkSubmaskParts = webServerData.networkSubmask.split(".");
    for (uint8_t i = 0; i < 4; i++) { networkSubmask[i] = networkSubmaskParts[i].trimmed(); }

    QStringList gatewayAddressParts = webServerData.gatewayAddress.split(".");
    for (uint8_t i = 0; i < 4; i++) { gatewayAddress[i] = gatewayAddressParts[i].trimmed(); }

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

void saveFailureLog(const QVector<Device *> &devices)
{
    QString folderPath = "fail";
    QDir dir;
    if (!dir.exists(folderPath)) { dir.mkpath(folderPath); } // make dir if missing

    QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString filePath = folderPath + "/" + currentDate + ".csv";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open fail log file: " << filePath;
        return;
    }

    QTextStream out(&file);
    out << "SubnetAddress;NodeSubnetAddress;RealAddress;SerialNumber;DeviceType;LampFail;CommunicationFail;DurationFail;BatteryFail;\n";

    for (const auto& device : devices) {
        if (!device->getIsConfigured()) continue;

        uint16_t realAddress = device->getRealAddress();
        int subnet = (realAddress - 1) / 64;
        int nodeSubnet =(realAddress - 1) % 64;

        QString uuidStr = device->serialNumberString();
        uint8_t deviceType = device->getDeviceType();
        bool lampFail = device->hasLampFailure();
        bool communicationFail = device->hasCommunicationFailure();
        bool durationFail = device->hasBatteryDurationFailure();
        bool batteryFail = device->hasBatteryFailure();

        out << subnet << ";" << nodeSubnet << ";" << realAddress << ";" << uuidStr << ";"
            << deviceType << ";" << lampFail << ";" << communicationFail << ";"
            << durationFail << ";" << batteryFail << ";\n";
    }
    qDebug() << "Failure log saved: " << filePath;
    file.close();
}

void saveTestLog(Database *database)
{
    QString folderPath = "test";
    QDir dir;
    if (!dir.exists(folderPath)) { dir.mkpath(folderPath); }// make dir if missing

    QString currentDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    QString filePath = folderPath + "/" + currentDate + ".csv";

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to open test log file: " << filePath;
        return;
    }

    QTextStream out(&file);
    out << "GroupAddress;FunctionalEnable;DurationEnable;FunctionalDays;FunctionalTime;DurationPeriodicity;DurationDate;DurationTime\n";

    QSqlQuery query;
    query.prepare("SELECT GroupAddress, FunctionalEnable, DurationEnable, FunctionalDays, FunctionalTime, DurationPeriodicity, DurationDate, DurationTime FROM Test");

    if (!query.exec()) {
        qDebug() << "Error executing SELECT query:" << query.lastError().text();
        file.close();
        return;
    }

    while (query.next()) {
        QString groupAddress = query.value(0).toString();
        bool functionalEnable = query.value(1).toBool();
        bool durationEnable = query.value(2).toBool();
        QString functionalDays = query.value(3).toString();
        QString functionalTime = query.value(4).toString();
        QString durationPeriodicity = query.value(5).toString();
        QString durationDate = query.value(6).toString();
        QString durationTime = query.value(7).toString();

        out << groupAddress << ";" << functionalEnable << ";" << durationEnable << ";"
            << functionalDays << ";" << functionalTime << ";"
            << durationPeriodicity << ";" << durationDate << ";" << durationTime << ";\n";
    }

    file.close();
    qDebug() << "Test log saved: " << filePath;
}

QString processLogFiles(QString folderPath, QDate start, QDate end, QTextStream &out, QString headerTitle)
{
    bool isFirstFile = true;

    for (QDate date = start; date <= end; date = date.addDays(1)) {
        QString filePath = folderPath + "/" + date.toString("yyyy-MM-dd") + ".csv";
        QFile inputFile(filePath);

        if (!inputFile.exists()) { continue; } // skip missing dates
        if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Failed to open file: " << filePath;
            continue;
        }

        QTextStream in(&inputFile);
        bool isFirstLine = true;

        if (isFirstFile) {
            out << "\n# " << headerTitle << "\n";
            isFirstFile = false;
        }

        out << "Date: " << date.toString("yyyy-MM-dd") << "\n";
        while (!in.atEnd()) {
            QString line = in.readLine();
            if ((!line.startsWith("SubnetAddress") && !line.startsWith("GroupAddress")) || isFirstLine) {
                out << line << "\n";
            }
            isFirstLine = false;
        }
        out << "\n";
        inputFile.close();
    }
    return "";
}

QString generateLogReport(QString reportType, QString startDate, QString endDate)
{
    QString folderPath;
    folderPath = (reportType == "fail") ? "fail" : "test";

    QString outputFileName = folderPath + "_report_" + startDate + "_to_" + endDate + ".csv";
    if (reportType == "all") {
        outputFileName = "all_report_" + startDate + "_to_" + endDate + ".csv"; // for "all" reports
    }

    QString logsDirPath = "/normagrup/www/logs/";
    QString outputFilePath = logsDirPath + outputFileName;

    QDir dir;
    if (!dir.exists(logsDirPath)) {
        if (!dir.mkpath(logsDirPath)) {
            qDebug() << "Failed to create logs directory: " << logsDirPath;
            return "";
        }
    }

    QFile outputFile(outputFilePath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Failed to create report file: " << outputFilePath;
        return "";
    }

    QTextStream out(&outputFile);
    out << "# Report Type: " << reportType.toUpper() << "\n";
    out << "# Date Range: " << startDate << " to " << endDate << "\n\n";

    QDate start = QDate::fromString(startDate, "yyyy-MM-dd");
    QDate end = QDate::fromString(endDate, "yyyy-MM-dd");

    if (reportType == "fail" || reportType == "all") {
        processLogFiles("fail", start, end, out, "Failure Reports");
    }

    if (reportType == "test" || reportType == "all") {
        processLogFiles("test", start, end, out, "Test Reports");
    }

    outputFile.close();
    return outputFileName; // return file name for webpage
}

