#include <QDebug>
#include <QRegularExpression>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QDate>


#include "file_handler.h"
#include "global_variables.h"
#include "Device.h"

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
    //  estado de LEDs
    if (QFile f1{"/sys/class/gpio/gpio82/value"}; f1.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QTextStream out(&f1); out << 1; f1.close();  // LINK OFF
    }
    if (QFile f2{"/sys/class/gpio/gpio81/value"}; f2.open(QIODevice::WriteOnly|QIODevice::Text)) {
        QTextStream out(&f2); out << 0; f2.close();  // FAIL ON
    }
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

QString exportLogToCSV(Database *db, const QString &type, QString startDate, QString endDate)
{
    QDate startQDate = QDate::fromString(startDate, "yyyy-MM-dd");
    QDate endQDate = QDate::fromString(endDate, "yyyy-MM-dd");

    if (!startQDate.isValid() || !endQDate.isValid()) { qDebug() << "Invalid date format!"; return ""; }

    QDateTime startDT(startQDate, QTime(0, 0, 0));
    QDateTime endDT(endQDate, QTime(23, 59, 59));

    qint64 start = startDT.toSecsSinceEpoch();
    qint64 end = endDT.toSecsSinceEpoch();

    QList<QStringList> logs = db->getLogEvent(type, start, end);
    transformEventCodes(&logs);

    QString outputFileName = type + "_report_" + startDate + "_to_" + endDate + ".csv";
    QString filePath = QString(LOG_DATA_PATH) + outputFileName;
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) { qDebug() << "Failed to open log file"; return ""; }

    QTextStream out(&file);
    out << "Name;Serial;BTAddress;IP;DateTime;Event;EventType\n";
    for (const QStringList &row : logs) {
        out << row.join(";") << ";\n";
    }
    file.close();
    return outputFileName;
}

void saveNetKeyAndMasterAddress(QString date, QString time, QString key, uint16_t masterRealAddress)
{
    QString fileName = "config_global.txt";
    QString filePath = QString(GENERAL_CONFIG_PATH) + fileName;

    QFile file(filePath);
    if (!file.open(QIODevice::Append | QIODevice::Text)) { qDebug() << "Failed to open config file"; return; }

    QTextStream out(&file);
    out << date << " " << time;
    out << " - ";
    out << "Antenna ID: " << (masterRealAddress - 31767) << " [" << masterRealAddress << "]";
    out << " - ";
    out << (key.size() == 32 ? "Custom NetKey: " : "NetKey ") << key;
    out << "\n";

    file.close();
}

void buildJsonTree()
{
    QJsonObject root = buildJsonTreeRecursively(0xC00F);
    QJsonDocument doc(root);

    QFile file(TREE_DATA_PATH);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    } else {
        qWarning() << "No se pudo escribir el JSON:" << file.errorString();
    }
}

QJsonObject buildJsonTreeRecursively(uint16_t realAddress)
{
    QString name;
    if(realAddress != 0xC00F) // La raíz no tiene nombre (no es un nodo como tal, es la antena)
    {
        const NodeInfo &node = nodesByRealAddress[realAddress];
        name = QString("Node %1 - %2")
                           .arg(node.subnetAddress * 16 + node.nodeSubnetAddress + 1)
                           .arg(node.serialNumber);
    }
    else
    {
        name = " ";
    }

    QJsonObject obj;
    obj["name"] = name;

    QList<uint16_t> children = childrenMap.values(realAddress);

    if (!children.isEmpty()) {
        obj["type"] = "folder";
        QJsonArray childrenArray;

        for (int childRealAddress : children) {
            childrenArray.append(buildJsonTreeRecursively(childRealAddress));
        }

        obj["children"] = childrenArray;
    } else {
        obj["type"] = "url";
    }

    return obj;
}

static bool readWholeFile(const QString& path, QByteArray* out) {
    QFile f(path);
    if (!f.exists()) return false;
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
    *out = f.readAll();
    return true;
}

static QDateTime readSystemUtc() {
    return QDateTime::currentDateTimeUtc();
}

static QDateTime readRtcFromSysfs()
{
    // /sys/class/rtc/rtc0/date -> YYYY-MM-DD
    // /sys/class/rtc/rtc0/time -> HH:MM:SS
    QByteArray dateRaw, timeRaw;
    if (!readWholeFile("/sys/class/rtc/rtc0/date", &dateRaw)) return {};
    if (!readWholeFile("/sys/class/rtc/rtc0/time", &timeRaw)) return {};

    const QString date = QString::fromUtf8(dateRaw).trimmed();
    const QString time = QString::fromUtf8(timeRaw).trimmed();

    const QDate d = QDate::fromString(date, "yyyy-MM-dd");
    const QTime t = QTime::fromString(time, "HH:mm:ss");
    if (!d.isValid() || !t.isValid()) return {};
    // El RTC de Linux normalmente está en UTC → créalo como UTC directamente
    return QDateTime(d, t, Qt::LocalTime);
}

static bool procRtcBatteryOK(bool* hasField)
{
    *hasField = false;
    QByteArray raw;
    if (!readWholeFile("/proc/driver/rtc", &raw)) return true; // si no existe, no penalizamos
    const QString s = QString::fromUtf8(raw);
    // Busca "battery : okay" o "battery : empty" si lo expone
    QRegularExpression rx("^battery\\s*:\\s*(\\w+)", QRegularExpression::MultilineOption);
    auto m = rx.match(s);
    if (!m.hasMatch()) return true;
    *hasField = true;
    const QString v = m.captured(1).toLower();
    return (v == "okay" || v == "good" || v == "ok");
}

QDateTime readRtcDateTime()
{
    QByteArray dateRaw, timeRaw;
    if (!readWholeFile("/sys/class/rtc/rtc0/date", &dateRaw)) return {};
    if (!readWholeFile("/sys/class/rtc/rtc0/time", &timeRaw)) return {};

    const QString date = QString::fromUtf8(dateRaw).trimmed(); // "YYYY-MM-DD"
    const QString time = QString::fromUtf8(timeRaw).trimmed(); // "HH:MM:SS"

    // Construir ISO en UTC explícito
    const QString iso = date + "T" + time + "Z";
    QDateTime dt = QDateTime::fromString(iso, Qt::ISODate);
    if (!dt.isValid()) return {};
    dt.setTimeSpec(Qt::UTC);            // redundante, pero explícito
    return dt;                          // ya es UTC correcto
}

bool rtcLooksUsable()
{
    const QDateTime rtc = readRtcDateTime();
    if (!rtc.isValid()) { qWarning() << "[RTC] inválido"; return false; }

    qInfo() << "[RTC] valor" << rtc.toString(Qt::ISODate) << "año" << rtc.date().year();
    if (rtc.date().year() < 2020) {
        qWarning() << "[RTC] año < 2020";
        return false;
    }

    QByteArray sinceRaw;
    if (readWholeFile("/sys/class/rtc/rtc0/since_epoch", &sinceRaw)) {
        bool ok = false;
        const qint64 since = QString::fromUtf8(sinceRaw).trimmed().toLongLong(&ok);
        qInfo() << "[RTC] since_epoch raw:" << sinceRaw << "ok?" << ok << "value:" << since;
        if (ok && since < 24*3600) {
            qWarning() << "[RTC] since_epoch < 1 día";
            return false;
        }
    } else {
        qInfo() << "[RTC] since_epoch no disponible";
    }

    bool hasBatt = false;
    const bool battOK = procRtcBatteryOK(&hasBatt);
    if (hasBatt && !battOK) {
        qWarning() << "[RTC] batería marcada como NO OK";
        return false;
    }
    return true;
}

void syncTimeFromRTCIfNeeded(int maxDriftSeconds)
{
    if (!rtcLooksUsable()) {
        qWarning() << "[RTC] No parece fiable (fecha/batería/epoch). No sincronizo desde RTC.";
        return;
    }

    const QDateTime rtc = readRtcDateTime();
    if (!rtc.isValid()) {
        qWarning() << "[RTC] No se pudo leer la hora del RTC.";
        return;
    }

    const QDateTime sys = readSystemUtc();
    const QDateTime cutoff(QDate(2015,1,1), QTime(0,0), Qt::UTC);
    const bool systemLooksWrong = (sys < cutoff);
    const qint64 drift = std::llabs(sys.secsTo(rtc));

    if (systemLooksWrong || drift > maxDriftSeconds) {
        QProcess p;
        //const QString epoch = QString::number(rtc.toSecsSinceEpoch());
        //p.start("sudo", {"timedatectl", "set-time", "@" + epoch});
        // Copia el RTC (UTC) al reloj del sistema sin ambigüedad
        p.start("sudo", {"hwclock", "--hctosys", "--utc", "-f", "/dev/rtc0"});
        p.waitForFinished(-1);
        //qInfo() << "[RTC] Sistema ajustado desde RTC (epoch=" << epoch << ")";
         qInfo() << "[RTC] Sistema ajustado desde RTC via hwclock --hctosys --utc";
    } else {
        qInfo() << "[RTC] Desfase" << drift << "s, no se ajusta.";
    }
}

void syncRTCFromSystem()
{
    QProcess p;
    // Escribir hora del sistema al RTC rk808 (UTC)
    p.start("sudo", {"hwclock", "-w", "--utc", "-f", "/dev/rtc0"});
    p.waitForFinished(-1);
    const int rc = p.exitCode();
    qInfo() << "[RTC] hwclock -w rc=" << rc << "stderr:" << p.readAllStandardError();
}
