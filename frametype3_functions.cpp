#include "frametype3_functions.h"
#include "aux_functions.h"

void sendDaliSpecialCommand(uint8_t subnet, uint8_t commandLow, uint8_t commandType)
{
    uint16_t targetAddress = getTargetAddress(subnet, 0xFF);
    // ENVIAR UART(COMMAND, COMMAND TYPE, TARGET ADDRESS);
}
