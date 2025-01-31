#include <QCoreApplication>
#include <QTime>

#include "time_functions.h"

void delay(uint16_t deadTimeMs)
{
    QTime deadTime = QTime::currentTime().addMSecs(deadTimeMs);
    while (QTime::currentTime() < deadTime) { QCoreApplication::processEvents(QEventLoop::AllEvents, 100); }
}
