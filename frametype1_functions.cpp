#include "frametype1_functions.h"
#include "aux_functions.h"
#include "process_uart_data.h"

void sendDaliCommand(UartPort* _uartPort, uint8_t daliMessageType, uint8_t subnet, uint8_t daliAddress, uint8_t commandLow, uint8_t commandType)
{
    uint16_t targetAddress = getTargetAddress(subnet, daliAddress);
    sendUartDaliCommand(_uartPort, daliMessageType, targetAddress, commandLow, commandType);
}
