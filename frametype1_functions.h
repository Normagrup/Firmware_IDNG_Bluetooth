#ifndef FRAMETYPE1_FUNCTIONS_H
#define FRAMETYPE1_FUNCTIONS_H

#include <QObject>
#include "UartPort.h"

#define DALI_NORMAL_TYPE            0x01
#define DALI_EXTENDED_TYPE          0x03
#define DALI_NORMAGROUP_TYPE        0x05

void sendDaliCommand(UartPort* _uartPort, uint8_t daliMessageType, uint8_t subnet, uint8_t daliAddress, uint8_t commandLow, uint8_t commandType);

#endif // FRAMETYPE1_FUNCTIONS_H
