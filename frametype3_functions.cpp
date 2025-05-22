#include "frametype3_functions.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "time_functions.h"

void sendDaliSpecialCommand(UartPort* _uartPort, uint8_t subnet, uint8_t daliAddress, uint8_t daliSpecialCmd, uint8_t value, uint8_t commandType)
{
    //uint16_t targetAddress = getTargetAddress(subnet, 0xFF);
    // ENVIAR UART(COMMAND, COMMAND TYPE, TARGET ADDRESS);
    if(daliAddress == 255){
        for (int node = 0; node < 64; ++node) {
            Device &device = meshDevice[subnet][node];
            if (device.getIsConfigured()) {
                uint16_t targetAddress = getTargetAddress(subnet, node);
                sendUartDaliCommand(_uartPort, targetAddress, daliSpecialCmd, value, commandType);
                delay(SLEEP_DALI_TIME_MS);
            }
        }
    } else {
        uint8_t nodesubnet = getNodeSubnetFromDaliAddress(daliAddress);
        uint16_t targetAddress = getTargetAddress(subnet, nodesubnet);
        sendUartDaliCommand(_uartPort, targetAddress, daliSpecialCmd, value, commandType);
    }
}
