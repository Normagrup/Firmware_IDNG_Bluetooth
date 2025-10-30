#ifndef FRAMETYPE3_FUNCTIONS_H
#define FRAMETYPE3_FUNCTIONS_H

#include <QObject>
#include "process_uart_data.h"


void sendDaliSpecialCommand(UartPort* _uartPort, uint8_t subnet, uint8_t daliAddress, uint8_t daliSpecialCmd, uint8_t value, uint8_t commandType);
void setDTR0FromEth(UartPort* _uartPort, uint8_t subnet, uint8_t daliAddress, uint8_t daliSpecialCmd, uint8_t value, uint8_t commandType);

#endif // FRAMETYPE3_FUNCTIONS_H
