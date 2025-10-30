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
            data[0] = rtcDateParts[0].right(2).toUInt();
            data[1] = rtcDateParts[1].toUInt();
            data[2] = rtcDateParts[2].toUInt();
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
            interfacesLines.append(interfacesInput.readLine());
        }
        while (interfacesLines.size() < 5)
            interfacesLines.append(QString());

        interfacesLines[4] = "gateway " + gatewayAddress;
        interfacesLines = interfacesLines.mid(0, 5);

        interfacesFile.resize(0);
        interfacesFile.seek(0);

        QTextStream interfacesOutput(&interfacesFile);
        for (const QString& line : interfacesLines) { interfacesOutput << line << '\n'; }

        interfacesOutput.flush();
        interfacesFile.flush();
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

// Convierte decimal [0..99] a BCD (entero 0..255)
static int decToBcd(int v) { return ((v/10)<<4) | (v%10); }

// Devuelve "0xHH"
static QString hex0x(int byte) {
    return QString("0x%1").arg(byte, 2, 16, QChar('0')).toLower();
}

// Lee hora del sistema tras el cambio (UTC) para verificar
static QDateTime readSystemUtc()
{
    return QDateTime::currentDateTimeUtc();
}

// Convierte "YYYY-MM-DD HH:MM:SS" a QDateTime válido
static QDateTime parseLocal(const QString& localDT)
{
    QDateTime dt = QDateTime::fromString(localDT, "yyyy-MM-dd HH:mm:ss");
    dt.setTimeSpec(Qt::LocalTime);
    return dt;
}

// Establece la hora
static bool setClockWithSyscall(const QDateTime& local)
{
    if (!local.isValid()) return false;

    QDateTime utc = local.toUTC();

#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    const qint64 sec  = utc.toSecsSinceEpoch();
    const long   nsec = utc.time().msec() * 1000000L;
#else
    const qint64 sec  = utc.toTime_t();
    const long   nsec = 0;
#endif

    struct timespec ts;
    ts.tv_sec  = sec;
    ts.tv_nsec = nsec;

    if (clock_settime(CLOCK_REALTIME, &ts) == 0) {
        qInfo() << "[SYSCLK] clock_settime OK (UTC):" << utc.toString("yyyy-MM-dd HH:mm:ss");
        return true;
    } else {
        qWarning() << "[SYSCLK] clock_settime fallo:" << strerror(errno) << "errno=" << errno;
        return false;
    }
}

// ult opción: usa /bin/date -s "YYYY-MM-DD HH:MM:SS"
static bool setClockWithDate(const QString& localDT, int timeoutMs = 5000)
{
    QProcess p;
    p.start("/bin/date", {"-s", localDT});
    if (!p.waitForFinished(timeoutMs)) {
        qWarning() << "[SYSCLK] date -s timeout; killed";
        p.kill();
        p.waitForFinished(1000);
        return false;
    }
    qInfo() << "[SYSCLK] date -s" << localDT
            << "exit:"   << p.exitCode()
            << "stderr:" << QString::fromLocal8Bit(p.readAllStandardError()).trimmed();
    return (p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0);
}

static bool setSystemClockLocal(const QString& localDT)
{
    const QDateTime before = readSystemUtc();

    // 1) Parsear local
    const QDateTime local = parseLocal(localDT);
    if (!local.isValid()) {
        qWarning() << "[SYSCLK] Fecha/hora LOCAL inválida:" << localDT;
        return false;
    }

    // 2) Intento 1: syscall directa (evita cuelgues de timedatectl/DBus)
    bool ok = setClockWithSyscall(local);

    // 3) Si no funciona lo intentamos con date -s
    if (!ok) ok = setClockWithDate(localDT);

    // 4) Verificación (≥ 1 s de diferencia).
    const QDateTime after = readSystemUtc();
    const bool changed = (qAbs(before.secsTo(after)) >= 1);

    qInfo() << "[SYSCLK] Resultado:" << (ok && changed ? "OK" : "FALLO")
            << "antes(UTC)="   << before.toString("yyyy-MM-dd HH:mm:ss")
            << "despues(UTC)=" << after.toString("yyyy-MM-dd HH:mm:ss");

    return ok && changed;
}

static void syncOnboardRtcFromSystem()
{
    // Actualiza el RTC del kernel (rk808) con la hora del sistema
    QString hw = QFile::exists("/sbin/hwclock") ? "/sbin/hwclock" : "/usr/sbin/hwclock";
    QProcess p;
    p.start(hw, {"--systohc"});   // system → hardware clock
    p.waitForFinished(3000);
    qInfo() << "[SYSCLK] hwclock --systohc exit:" << p.exitCode()
            << "stderr:" << QString::fromLocal8Bit(p.readAllStandardError()).trimmed();
}

void setLocalDateTime(QStringList dateTimeParts)
{
    const QString date = dateTimeParts.value(0);           // "YYYY-MM-DD"
    const QString time = dateTimeParts.value(1) + ":00";   // "HH:MM:SS"
    const QString dateTime = date + " " + time;

    const bool sysOk = setSystemClockLocal(dateTime);
    if (!sysOk) {
        qWarning() << "[SYSCLK] No se pudo ajustar la hora del sistema.";
    }
    syncOnboardRtcFromSystem();

    // 2) Convierte esa hora local a UTC (RTC en UTC recomendado)
    QDateTime local = QDateTime::fromString(dateTime, "yyyy-MM-dd HH:mm:ss");
    local.setTimeSpec(Qt::LocalTime);
    const QDateTime utc = local.toUTC();

    const QDate d = utc.date();
    const QTime t = utc.time();

    const int yy = d.year() % 100;          // 00..99
    int wday = d.dayOfWeek() % 7;           // Qt: 1=Mon..7=Sun → %7: Sun=0
    if (wday < 0) wday = 0;

    // 3) BCD de cada campo (seg con bit7=0 → limpia OSF)
    const int bSS = decToBcd(t.second() & 0x7F);
    const int bMM = decToBcd(t.minute());
    const int bHH = decToBcd(t.hour());
    const int bDD = decToBcd(d.day());
    const int bWK = decToBcd(wday);
    const int bMO = decToBcd(d.month());
    const int bYY = decToBcd(yy);

    // 4) Escribir en el PCF con i2c-tools (replica EXACTA del comando que te funciona)
    {
        static const QString I2CTRANSFER = "/usr/sbin/i2ctransfer";
        static const QString I2CSET      = "/usr/sbin/i2cset";     // por si quieres STOP=1/0

        // Construimos los mismos argumentos que tu comando manual:
        // i2ctransfer -y 1 w8@0x51 0x04  0xSS 0xMM 0xHH 0xDD 0xWK 0xMO 0xYY
        QStringList args;
        args << "-y" << "1"
             << "w8@0x51"
             << "0x04"
             << hex0x(bSS) << hex0x(bMM) << hex0x(bHH)
             << hex0x(bDD) << hex0x(bWK) << hex0x(bMO) << hex0x(bYY);

        // Log del comando exacto (para que veas qué se ejecuta):
        qInfo() << "[RTC] write cmd:" << I2CTRANSFER << args;

        QProcess p2;
        p2.start(I2CTRANSFER, args);
        p2.waitForFinished(-1);

        const QByteArray out = p2.readAllStandardOutput();
        const QByteArray err = p2.readAllStandardError();
        if (!out.isEmpty()) qInfo()  << "[RTC] i2ctransfer stdout:" << out.trimmed();
        if (!err.isEmpty()) qWarning() << "[RTC] i2ctransfer stderr:" << err.trimmed();

        if (p2.exitStatus()!=QProcess::NormalExit || p2.exitCode()!=0) {
            qWarning() << "i2ctransfer write failed (code" << p2.exitCode() << ")";
            // Fallback “forzado” con shell por si el PATH/capabilities molestan:
            QProcess pf;
            QString oneLine = I2CTRANSFER + " -y 1 w8@0x51 0x04 "
                              + hex0x(bSS) + " " + hex0x(bMM) + " " + hex0x(bHH) + " "
                              + hex0x(bDD) + " " + hex0x(bWK) + " " + hex0x(bMO) + " " + hex0x(bYY);
            pf.start("/bin/sh", {"-c", oneLine});
            pf.waitForFinished(-1);
            qInfo() << "[RTC] fallback shell exit:" << pf.exitCode()
                    << "stderr:" << pf.readAllStandardError().trimmed();
        }

        // Reanudar reloj: CONTROL1 STOP=0
        {
            QProcess p;
            p.start(I2CSET, {"-y","1","0x51","0x00","0x00"});
            p.waitForFinished(-1);
            if (p.exitCode()!=0)
                qWarning() << "i2cset STOP=0 failed:" << p.readAllStandardError();
        }
    }

    qInfo() << "RTC (PCF85063A) actualizado a (UTC):" << utc.toString("yyyy-MM-dd HH:mm:ss");
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

void getBuildingNameDB(Database *database, uint8_t *data)
{
    QString buildingName = database->getGeneralData("BuildingName").trimmed().left(16);
    QByteArray byteArray = buildingName.toLatin1();

    for (int i = 0; i < byteArray.size(); i++)
        data[i] = byteArray[i];
    for (int i = byteArray.size(); i < 16; i++)
        data[i] = 0;
}

void getLineNameDB(Database *database, uint8_t *data)
{
    QString lineName = database->getGeneralData("LineName").trimmed().left(16);
    QByteArray byteArray = lineName.toLatin1();

    for (int i = 0; i < byteArray.size(); i++)
        data[i] = byteArray[i];
    for (int i = byteArray.size(); i < 16; i++)
        data[i] = 0;
}
