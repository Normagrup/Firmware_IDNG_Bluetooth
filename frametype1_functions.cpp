#include "frametype1_functions.h"
#include "aux_functions.h"
#include "process_uart_data.h"
#include "dali_headers.h"
#include "time_functions.h"
#include "Database.h"

Database* database;

void sendDaliCommand(UartPort* _uartPort, uint8_t daliMessageType, uint8_t subnet, uint8_t daliAddress, uint8_t commandLow, uint8_t commandType)
{
    //sendUartDaliCommand(_uartPort, daliMessageType, targetAddress, commandLow, commandType);
    if(daliAddress == 255){
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        if(subnet == 255){//All device
            for(int i = 0; i < MAX_SUBNET; i++){
                for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                    Device& device = meshDevice[i][j];
                    if(device.getIsConfigured()){
                        if(commandLow == RECALL_MAX_LVL)
                            device.setActualLvl(254);
                        else if(commandLow == RECALL_MIN_LVL)
                            device.setActualLvl(3);
                        else if (commandLow == OFF)
                            device.setActualLvl(0);
                    }
                }
            }
            sendUartDaliCommand(_uartPort, 0xFFFF, BROADCAST_ADDR, commandLow, commandType);
            delay(SLEEP_DALI_TIME_MS);
        } else { //subnet control
            for (int node = 0; node < 64; ++node) {
                Device &device = meshDevice[subnet][node];
                if (device.getIsConfigured()) {
                    uint16_t targetAddress = getTargetAddress(subnet, node);
                    if(commandLow == RECALL_MAX_LVL)
                        device.setActualLvl(254);
                    else if(commandLow == RECALL_MIN_LVL)
                        device.setActualLvl(3);
                    else if (commandLow == OFF)
                        device.setActualLvl(0);
                    sendUartDaliCommand(_uartPort, targetAddress, BROADCAST_ADDR, commandLow, commandType);
                    delay(SLEEP_DALI_TIME_MS);
                }
            }
        }
    } else if (subnet == 255 && daliAddress > 32 && daliAddress < 255) { //Group control
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        uint16_t targetAddress = getGroupAddressFromDaliAddress(daliAddress);
        if(daliMessageType == 3){
            sendUartDaliCommand(_uartPort, targetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
        }

        for(int i = 0; i < MAX_SUBNET; i++){
            for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                Device& device = meshDevice[i][j];
                if(device.isOnGroupSubAddress(targetAddress)){
                    if(commandLow == RECALL_MAX_LVL)
                        device.setActualLvl(254);
                    else if(commandLow == RECALL_MIN_LVL)
                        device.setActualLvl(3);
                    else if (commandLow == OFF)
                        device.setActualLvl(0);
                }
            }
        }

        sendUartDaliCommand(_uartPort, targetAddress, BROADCAST_ADDR, commandLow, commandType);
        if(commandLow == 227 || commandLow == 228 || commandLow == 229){
            logTestRequest(database, targetAddress, true, logTestTypeHelper(commandLow));
        }

    } else { //device control
        cleanCdbTimer.start(TIME_TO_CLEAN_CDB);
        uint8_t nodesubnet = getNodeSubnetFromDaliAddress(daliAddress);
        uint16_t targetAddress = getTargetAddress(subnet, nodesubnet);
        if(daliMessageType == 3){
            sendUartDaliCommand(_uartPort, targetAddress, ENABLE_DEVICE_TYPE, 0x01, IS_NORMAL);
            delay(SLEEP_DALI_TIME_MS);
        }

        for(int i = 0; i < MAX_SUBNET; i++){
            for(int j = 0; j < MAX_NODES_SUBNET; j++) {
                Device& device = meshDevice[i][j];
                if(device.getRealAddress() == targetAddress){
                    if(commandLow == RECALL_MAX_LVL)
                        device.setActualLvl(254);
                    else if(commandLow == RECALL_MIN_LVL)
                        device.setActualLvl(3);
                    else if (commandLow == OFF)
                        device.setActualLvl(0);
                }
            }
        }

        sendUartDaliCommand(_uartPort, targetAddress, BROADCAST_ADDR, commandLow, commandType);
        if(commandLow == 227 || commandLow == 228 || commandLow == 229){
            int globalPos = subnet * 64 + nodesubnet + 1;
            logTestRequest(database, globalPos, false, logTestTypeHelper(commandLow));
        }
    }
}

void askPowerOnLevelFromEthToDali(UartPort* _uartPort, uint8_t subnet, uint8_t daliAddress)
{
    uint16_t targetAddress = getTargetAddress(subnet, daliAddress);

    QByteArray frame;
    unsigned char length = 5;

    frame.append(UART_HEADER);
    frame.append(length);
    frame.append(UART_CONFIG_FRAME_TYPE);
    frame.append(ASK_POWER_ON_LEVEL);
    frame.append((targetAddress >> 8) & 0xFF);
    frame.append(targetAddress & 0xFF);
    frame.append(UART_END);

    _uartPort->sendData(frame);

    delay(SLEEP_DALI_TIME_MS * 2);
}
