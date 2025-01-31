#include <QCoreApplication>
#include <QDebug>

#include "Wireless.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << "App running...";

    Wireless* wirelessNet = new Wireless(nullptr);
    wirelessNet->runNetwork();

    return a.exec();
}
