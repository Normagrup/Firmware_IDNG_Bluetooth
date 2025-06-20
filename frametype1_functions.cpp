#include "frametype1_functions.h"
#include "aux_functions.h"
#include "process_uart_data.h"
#include "dali_headers.h"
#include "time_functions.h"

void sendDaliCommand(UartPort* _uartPort, uint8_t daliMessageType, uint8_t subnet, uint8_t daliAddress, uint8_t commandLow, uint8_t commandType)
{
    //sendUartDaliCommand(_uartPort, daliMessageType, targetAddress, commandLow, commandType);
    if(daliAddress == 255){
        for (int node = 0; node < 64; ++node) {
            Device &device = meshDevice[subnet][node];
            if (device.getIsConfigured()) {
                uint16_t targetAddress = getTargetAddress(subnet, node);
                if(daliMessageType == 3){
                    sendUartDaliCommand(_uartPort, targetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
                    delay(SLEEP_DALI_TIME_MS);
                }
                sendUartDaliCommand(_uartPort, targetAddress, BROADCAST_ADDR, commandLow, commandType);
                delay(SLEEP_DALI_TIME_MS);
            }
        }
    } else if (subnet == 255 && daliAddress > 32) {
        uint16_t targetAddress = getGroupAddressFromDaliAddress(daliAddress);
        if(daliMessageType == 3){
            sendUartDaliCommand(_uartPort, targetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
        }
        sendUartDaliCommand(_uartPort, targetAddress, BROADCAST_ADDR, commandLow, commandType);
    } else {
        uint8_t nodesubnet = getNodeSubnetFromDaliAddress(daliAddress);
        uint16_t targetAddress = getTargetAddress(subnet, nodesubnet);
        if(daliMessageType == 3){
            sendUartDaliCommand(_uartPort, targetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
        }
        sendUartDaliCommand(_uartPort, targetAddress, BROADCAST_ADDR, commandLow, commandType);
    }
}
