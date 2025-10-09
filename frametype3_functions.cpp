#include "frametype3_functions.h"
#include "aux_functions.h"
#include "dali_headers.h"
#include "time_functions.h"

void sendDaliSpecialCommand(UartPort* _uartPort, uint8_t subnet, uint8_t daliAddress, uint8_t daliSpecialCmd, uint8_t value, uint8_t commandType)
{
    //uint16_t targetAddress = getTargetAddress(subnet, 0xFF);
    // ENVIAR UART(COMMAND, COMMAND TYPE, TARGET ADDRESS);
    if(daliAddress == 255){
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        if(subnet == 255){ //All device
            for(int i = 0; i < MAX_SUBNET; i++){
                for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                    Device& device = meshDevice[i][j];
                    if(device.getIsConfigured()){
                        if(daliSpecialCmd == ARC_POWER_DAPC)
                            device.setActualLvl(value);
                    }
                }
            }
            sendUartDaliCommand(_uartPort, 0xFFFF, daliSpecialCmd, value, commandType);
            delay(SLEEP_DALI_TIME_MS);
        } else { //subnet control
            for (int node = 0; node < 64; ++node) {
                Device &device = meshDevice[subnet][node];
                if (device.getIsConfigured()) {
                    uint16_t targetAddress = getTargetAddress(subnet, node);
                    if(daliSpecialCmd == ARC_POWER_DAPC)
                        device.setActualLvl(value);
                    sendUartDaliCommand(_uartPort, targetAddress, daliSpecialCmd, value, commandType);
                    delay(SLEEP_DALI_TIME_MS);
                }
            }
        }
    } else if (subnet == 255 && daliAddress > 32 && daliAddress < 255 ) {  //group control
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        uint16_t targetAddress = getGroupAddressFromDaliAddress(daliAddress);
        for(int i = 0; i < MAX_SUBNET; i++){
            for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                Device& device = meshDevice[i][j];
                if(device.isOnGroupSubAddress(targetAddress)){
                    if(daliSpecialCmd == ARC_POWER_DAPC)
                        device.setActualLvl(value);
                }
            }
        }
        sendUartDaliCommand(_uartPort, targetAddress,  daliSpecialCmd, value, commandType);

    } else { //single device
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        uint8_t nodesubnet = getNodeSubnetFromDaliAddress(daliAddress);
        uint16_t targetAddress = getTargetAddress(subnet, nodesubnet);

        for(int i = 0; i < MAX_SUBNET; i++){
            for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                Device& device = meshDevice[i][j];
                if(device.getRealAddress() == targetAddress){
                    if(daliSpecialCmd == ARC_POWER_DAPC)
                        device.setActualLvl(value);
                }
            }
        }

        sendUartDaliCommand(_uartPort, targetAddress, daliSpecialCmd, value, commandType);
    }
}


void setDTR0FromEth(UartPort *_uartPort, uint8_t subnet, uint8_t daliAddress, uint8_t daliSpecialCmd, uint8_t value, uint8_t commandType)
{
    cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
    uint16_t targetAddress = getMaskedGroupId(subnet);
    sendUartDaliCommand(_uartPort, targetAddress, daliSpecialCmd, value, commandType);
}
