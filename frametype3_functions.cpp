#include "frametype3_functions.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "time_functions.h"

void sendDaliSpecialCommand(UartPort* _uartPort, uint8_t subnet, uint8_t daliAddress, uint8_t daliSpecialCmd, uint8_t value, uint8_t commandType)
{
    //uint16_t targetAddress = getTargetAddress(subnet, 0xFF);
    // ENVIAR UART(COMMAND, COMMAND TYPE, TARGET ADDRESS);
    if(daliAddress == 255){ //subnet control
        for (int node = 0; node < 64; ++node) {
            Device &device = meshDevice[subnet][node];
            if (device.getIsConfigured()) {
                uint16_t targetAddress = getTargetAddress(subnet, node);
                sendUartDaliCommand(_uartPort, targetAddress, daliSpecialCmd, value, commandType);
                delay(SLEEP_DALI_TIME_MS);
            }
        }
    } else if (subnet == 255 && daliAddress > 32) {  //group control
        uint16_t targetAddress = getGroupAddressFromDaliAddress(daliAddress);
        sendUartDaliCommand(_uartPort, targetAddress,  daliSpecialCmd, value, commandType);

    } else { //single device
        uint8_t nodesubnet = getNodeSubnetFromDaliAddress(daliAddress);
        uint16_t targetAddress = getTargetAddress(subnet, nodesubnet);
        sendUartDaliCommand(_uartPort, targetAddress, daliSpecialCmd, value, commandType);
    }
}


void setDTR0FromEth(UartPort *_uartPort, uint8_t subnet, uint8_t daliAddress, uint8_t daliSpecialCmd, uint8_t value, uint8_t commandType)
{
    uint16_t targetAddress = getMaskedGroupId(subnet);
    sendUartDaliCommand(_uartPort, targetAddress, daliSpecialCmd, value, commandType);
}
