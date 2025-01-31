#include <QObject>
#include <QDebug>

#include "process_eth_data.h"
#include "eth_frames.h"
#include "global_variables.h"
#include "file_handler.h"
#include "frametype0_functions.h"
#include "frametype1_functions.h"
#include "frametype3_functions.h"
#include "frametype4_functions.h"
#include "dali_headers.h"

bool checkCRC(QByteArray data)
{
    uint8_t checkSum, crc = 0;

    for (uint8_t i = 3; i < (data.length() - 1); i++) { crc += data[i]; }

    checkSum = (unsigned char)data[data.length() - 1];

    if (crc == checkSum) { return true; }
    else { return false; }
}

bool checkFrameHeader(QByteArray data)
{
    if ((unsigned char)data[0] == FRAME_HEADER_0 && (unsigned char)data[1] == FRAME_HEADER_1 && (unsigned char)data[2] == FRAME_HEADER_2) { return true; }
    else { return false; }
}

bool checkRcvAddress(QString rcvAddress)
{
    QStringList rcvAddressParts = rcvAddress.split(".");
    QString defaultSubmaskAddress = DEFAULT_SUBMASK_ADDRESS;
    QStringList defaultSubmaskAddressParts = defaultSubmaskAddress.split(".");

    if ((networkIP[0].toUInt() & defaultSubmaskAddressParts[0].trimmed().toUInt()) == (rcvAddressParts[0].trimmed().toUInt() & defaultSubmaskAddressParts[0].trimmed().toUInt()) ||
            (networkIP[1].toUInt() & defaultSubmaskAddressParts[1].trimmed().toUInt()) == (rcvAddressParts[1].trimmed().toUInt() & defaultSubmaskAddressParts[1].trimmed().toUInt()) ||
            (networkIP[2].toUInt() & defaultSubmaskAddressParts[2].trimmed().toUInt()) == (rcvAddressParts[2].trimmed().toUInt() & defaultSubmaskAddressParts[2].trimmed().toUInt()) ||
            (networkIP[3].toUInt() & defaultSubmaskAddressParts[3].trimmed().toUInt()) == (rcvAddressParts[3].trimmed().toUInt() & defaultSubmaskAddressParts[3].trimmed().toUInt())) {
        return true;
    }
    else { return false; }
}

static void processEthFrameType0(QString rcvAddress, QByteArray data, UdpSocket* _udpSocket)
{
    uint8_t commandHigh = (unsigned char)data[7];
    uint8_t commandLow = (unsigned char)data[8];

    switch (commandHigh) {
    case 0x00:
        switch (commandLow) {
        case 0x00: // GET IP ADDRESS
            sendIPAddressFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x02: // GET SUBMASK ADDRESS
            sendSubmaskAddressFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x04: // GET GATEWAY ADDRESS
            sendGatewayAddressFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x08: // GET MAC ADDRESS
            sendMacAddressFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x10: // GET BUILDING NAME
            sendBuildingNameFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x20: // GET LINE NAME
            sendLineNameFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x30: // GET RTC DATE DAY
            sendRtcDateDayFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x32: // GET RTC DATE SECONDS
            sendRtcDateDaySeconds(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x40: // GET MODEL VERSION
            sendModelVersionFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x42: // GET FIRMWARE VERSION
            sendFirmwareVersionFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x50: // GET ADMIN PASSWORD
            sendAdminPasswordFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x54: // GET MANTENEDOR PASSWORD
            sendMantenedorPasswordFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x80: // GET HISTORIC READABLE EVENTS
            // ???????
            break;

        case 0x88: // HISTORIC RESET
            // ???????
            break;

        default:
            break;
        }
        break;

    case 0x01:
        switch (commandLow) {
        case 0x00: // REBOOT
            sendAckFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            rebootDevice();
            break;

        case 0x01: // RESET ETHERNET CONFIG
            // ???????
            break;

        case 0xFF: // PING
            sendPingFrame(rcvAddress, (unsigned char)data[5], _udpSocket);
            break;

        default:
            break;
        }
        break;

    case 0x40:
        switch (commandLow) {
        case 0x00: // DALI LINE START SCANNING
            // ???????
            break;

        case 0x40: // DALI LINE READ INSTALLED DEVICES
            if ((unsigned char)data[4] == 0xFF) { sendDaliLineInstalledDevices(rcvAddress, commandHigh, commandLow, _udpSocket); }
            else {}
            break;

        case 0x41: // DALI LINE READ DEVICES TYPE
            if ((unsigned char)data[4] == 0xFF) { sendDaliLineReadDevicesType(rcvAddress, commandHigh, commandLow, _udpSocket); }
            else {}
            break;

        case 0x42: // DALI LINE READ COMUNICATION STATE
            if ((unsigned char)data[4] == 0xFF) { sendDaliLineReadComunicationState(rcvAddress, commandHigh, commandLow, _udpSocket); }
            else {}
            break;

        case 0x60: // DALI LINE QUUERY ACTUAL LVL
            if ((unsigned char)data[4] == 0xFF) { sendDaliLineQueryActualLvl(rcvAddress, commandHigh, commandLow, _udpSocket); }
            else {}
            break;

        case 0x61: // DALI LINE QUERY STATUS
            if ((unsigned char)data[4] == 0xFF) { sendDaliLineQueryStatus(rcvAddress, commandHigh, commandLow, _udpSocket); }
            else {}
            break;

        case 0x80: // DALI LINE QUERY EMERGENCY MODE
            if ((unsigned char)data[4] == 0xFF) { sendDaliLineQueryEmergencyMode(rcvAddress, commandHigh, commandLow, _udpSocket); }
            else {}
            break;

        case 0x81: // DALI LINE QUERY FAILURE STATUS
            if ((unsigned char)data[4] == 0xFF) { sendDaliLineQueryFailureStatus(rcvAddress, commandHigh, commandLow, _udpSocket); }
            else {}
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

static void processEthFrameType1(QString rcvAddress, QByteArray data, UdpSocket* _udpSocket, UartPort* _uartPort)
{
    uint8_t commandHigh = (unsigned char)data[8];
    uint8_t commandLow = (unsigned char)data[9];

    uint8_t subnet = (unsigned char)data[4];
    uint8_t daliAddress = (unsigned char)data[7];

    switch (commandHigh) {
    case 0x63:
        switch (commandLow) {
        case 0x01: // OFF
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, OFF, IS_NORMAL);
            break;

        case 0x02: // UP
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, UP, IS_NORMAL);
            break;

        case 0x03: // DOWN
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, DOWN, IS_NORMAL);
            break;

        case 0x04: // STEP UP
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STEP_UP, IS_NORMAL);
            break;

        case 0x05: // STEP DOWN
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STEP_DOWN, IS_NORMAL);
            break;

        case 0x06: // RECALL MAX LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, RECALL_MAX_LVL, IS_NORMAL);
            break;

        case 0x07: // RECALL MIN LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, RECALL_MIN_LVL, IS_NORMAL);
            break;

        case 0x08: // STEP DOWN AND OFF
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STEP_DOWN_AND_OFF, IS_NORMAL);
            break;

        case 0x09: // ON AND STEP UP
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ON_AND_STEP_UP, IS_NORMAL);
            break;

        case 0x10: // ENABLE DACP SEQUENCE
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ENABLE_DAPC_SEQUENCE, IS_NORMAL);
            break;

        case 0x11: // GO TO SCENE 0
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_0, IS_NORMAL);
            break;

        case 0x12: // GO TO SCENE 1
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_1, IS_NORMAL);
            break;

        case 0x13: // GO TO SCENE 2
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_2, IS_NORMAL);
            break;

        case 0x14: // GO TO SCENE 3
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_3, IS_NORMAL);
            break;

        case 0x15: // GO TO SCENE 4
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_4, IS_NORMAL);
            break;

        case 0x16: // GO TO SCENE 5
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_5, IS_NORMAL);
            break;

        case 0x17: // GO TO SCENE 6
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_6, IS_NORMAL);
            break;

        case 0x18: // GO TO SCENE 7
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_7, IS_NORMAL);
            break;

        case 0x19: // GO TO SCENE 8
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_8, IS_NORMAL);
            break;

        case 0x1A: // GO TO SCENE 9
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_9, IS_NORMAL);
            break;

        case 0x1B: // GO TO SCENE 10
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_10, IS_NORMAL);
            break;

        case 0x1C: // GO TO SCENE 11
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_11, IS_NORMAL);
            break;

        case 0x1D: // GO TO SCENE 12
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_12, IS_NORMAL);
            break;

        case 0x1E: // GO TO SCENE 13
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_13, IS_NORMAL);
            break;

        case 0x1F: // GO TO SCENE 14
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_14, IS_NORMAL);
            break;

        case 0x20: // GO TO SCENE 15
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, GO_SCENE_15, IS_NORMAL);
            break;

        case 0x21: // RESET
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, RESET, IS_TWICE);
            break;

        case 0x22: // STORE ACTUAL LVL IN DTR
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_ACTUAL_LVL_DTR, IS_TWICE);
            break;

        case 0x2B: // STORE DTR AS MAX LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_MAX_LVL, IS_TWICE);
            break;

        case 0x2C: // STORE DTR AS MIN LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_MIN_LVL, IS_TWICE);
            break;

        case 0x2D: // STORE DTR AS SYSTEM FAILURE LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SYS_FAILURE_LVL, IS_TWICE);
            break;

        case 0x2E: // STORE DTR AS POWER ON LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_POWER_ON_LVL, IS_TWICE);
            break;

        case 0x2F: // STORE DTR AS FADE TIME
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_FADE_TIME, IS_TWICE);
            break;

        case 0x30: // STORE DTR AS FADE RATE
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_FADE_RATE, IS_TWICE);
            break;

        case 0x41: // STORE DTR AS SCENE 0
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_0, IS_TWICE);
            break;

        case 0x42: // STORE DTR AS SCENE 1
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_1, IS_TWICE);
            break;

        case 0x43: // STORE DTR AS SCENE 2
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_2, IS_TWICE);
            break;

        case 0x44: // STORE DTR AS SCENE 3
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_3, IS_TWICE);
            break;

        case 0x45: // STORE DTR AS SCENE 4
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_4, IS_TWICE);
            break;

        case 0x46: // STORE DTR AS SCENE 5
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_5, IS_TWICE);
            break;

        case 0x47: // STORE DTR AS SCENE 6
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_6, IS_TWICE);
            break;

        case 0x48: // STORE DTR AS SCENE 7
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_7, IS_TWICE);
            break;

        case 0x49: // STORE DTR AS SCENE 8
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_8, IS_TWICE);
            break;

        case 0x4A: // STORE DTR AS SCENE 9
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_9, IS_TWICE);
            break;

        case 0x4B: // STORE DTR AS SCENE 10
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_10, IS_TWICE);
            break;

        case 0x4C: // STORE DTR AS SCENE 11
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_11, IS_TWICE);
            break;

        case 0x4D: // STORE DTR AS SCENE 12
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_12, IS_TWICE);
            break;

        case 0x4E: // STORE DTR AS SCENE 13
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_13, IS_TWICE);
            break;

        case 0x4F: // STORE DTR AS SCENE 14
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_14, IS_TWICE);
            break;

        case 0x50: // STORE DTR AS SCENE 15
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SCENE_15, IS_TWICE);
            break;

        case 0x51: // REMOVE FROM SCENE 0
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_0, IS_TWICE);
            break;

        case 0x52: // REMOVE FROM SCENE 1
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_1, IS_TWICE);
            break;

        case 0x53: // REMOVE FROM SCENE 2
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_2, IS_TWICE);
            break;

        case 0x54: // REMOVE FROM SCENE 3
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_3, IS_TWICE);
            break;

        case 0x55: // REMOVE FROM SCENE 4
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_4, IS_TWICE);
            break;

        case 0x56: // REMOVE FROM SCENE 5
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_5, IS_TWICE);
            break;

        case 0x57: // REMOVE FROM SCENE 6
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_6, IS_TWICE);
            break;

        case 0x58: // REMOVE FROM SCENE 7
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_7, IS_TWICE);
            break;

        case 0x59: // REMOVE FROM SCENE 8
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_8, IS_TWICE);
            break;

        case 0x5A: // REMOVE FROM SCENE 9
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_9, IS_TWICE);
            break;

        case 0x5B: // REMOVE FROM SCENE 10
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_10, IS_TWICE);
            break;

        case 0x5C: // REMOVE FROM SCENE 11
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_11, IS_TWICE);
            break;

        case 0x5D: // REMOVE FROM SCENE 12
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_12, IS_TWICE);
            break;

        case 0x5E: // REMOVE FROM SCENE 13
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_13, IS_TWICE);
            break;

        case 0x5F: // REMOVE FROM SCENE 14
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_14, IS_TWICE);
            break;

        case 0x60: // REMOVE FROM SCENE 15
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_SCENE_15, IS_TWICE);
            break;

        case 0x61: // ADD TO GROUP 0
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_0, IS_TWICE);
            break;

        case 0x62: // ADD TO GROUP 1
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_1, IS_TWICE);
            break;

        case 0x63: // ADD TO GROUP 2
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_2, IS_TWICE);
            break;

        case 0x64: // ADD TO GROUP 3
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_3, IS_TWICE);
            break;

        case 0x65: // ADD TO GROUP 4
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_4, IS_TWICE);
            break;

        case 0x66: // ADD TO GROUP 5
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_5, IS_TWICE);
            break;

        case 0x67: // ADD TO GROUP 6
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_6, IS_TWICE);
            break;

        case 0x68: // ADD TO GROUP 7
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_7, IS_TWICE);
            break;

        case 0x69: // ADD TO GROUP 8
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_8, IS_TWICE);
            break;

        case 0x6A: // ADD TO GROUP 9
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_9, IS_TWICE);
            break;

        case 0x6B: // ADD TO GROUP 10
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_10, IS_TWICE);
            break;

        case 0x6C: // ADD TO GROUP 11
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_11, IS_TWICE);
            break;

        case 0x6D: // ADD TO GROUP 12
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_12, IS_TWICE);
            break;

        case 0x6E: // ADD TO GROUP 13
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_13, IS_TWICE);
            break;

        case 0x6F: // ADD TO GROUP 14
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_14, IS_TWICE);
            break;

        case 0x70: // ADD TO GROUP 15
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ADD_GROUP_15, IS_TWICE);
            break;

        case 0x71: // REMOVE FROM GROUP 0
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_0, IS_TWICE);
            break;

        case 0x72: // REMOVE FROM GROUP 1
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_1, IS_TWICE);
            break;

        case 0x73: // REMOVE FROM GROUP 2
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_2, IS_TWICE);
            break;

        case 0x74: // REMOVE FROM GROUP 3
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_3, IS_TWICE);
            break;

        case 0x75: // REMOVE FROM GROUP 4
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_4, IS_TWICE);
            break;

        case 0x76: // REMOVE FROM GROUP 5
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_5, IS_TWICE);
            break;

        case 0x77: // REMOVE FROM GROUP 6
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_6, IS_TWICE);
            break;

        case 0x78: // REMOVE FROM GROUP 7
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_7, IS_TWICE);
            break;

        case 0x79: // REMOVE FROM GROUP 8
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_8, IS_TWICE);
            break;

        case 0x7A: // REMOVE FROM GROUP 9
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_9, IS_TWICE);
            break;

        case 0x7B: // REMOVE FROM GROUP 10
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_10, IS_TWICE);
            break;

        case 0x7C: // REMOVE FROM GROUP 11
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_11, IS_TWICE);
            break;

        case 0x7D: // REMOVE FROM GROUP 12
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_12, IS_TWICE);
            break;

        case 0x7E: // REMOVE FROM GROUP 13
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_13, IS_TWICE);
            break;

        case 0x7F: // REMOVE FROM GROUP 14
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_14, IS_TWICE);
            break;

        case 0x80: // REMOVE FROM GROUP 15
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, REMOVE_FROM_GROUP_15, IS_TWICE);
            break;

        case 0x81: // STORE DTR AS SHORT ADDRESS
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, STORE_DTR_SHORT_ADDRESS, IS_TWICE);
            break;

        case 0x82: // ENABLE WRITE MEMORY
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, ENABLE_WRITE_MEMORY, IS_TWICE);
            break;

        case 0x91: // QUERY STATUS
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_STATUS, IS_QUERY);
            break;

        case 0x92: // QUERY CONTROL GEAR
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_CONTROL_GEAR, IS_QUERY);
            break;

        case 0x93: // QUERY LAMP FAILURE
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_LAMP_FAILURE, IS_QUERY);
            break;

        case 0x94: // QUERY LAMP POWER ON
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_LAMP_POWER_ON, IS_QUERY);
            break;

        case 0x95: // QUERY LIMIT ERROR
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_LIMIT_ERROR, IS_QUERY);
            break;

        case 0x96: // QUERY RESET STATE
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_RESET_STATE, IS_QUERY);
            break;

        case 0x97: // QUERY MISSING SHORT ADDRESS
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_MISSING_SHORT_ADDRESS, IS_QUERY);
            break;

        case 0x98: // QUERY VERSION NUMBER
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_VERSION_NUMBER, IS_QUERY);
            break;

        case 0x99: // QUERY CONTENT DTR0
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_CONTENT_DTR_0, IS_QUERY);
            break;

        case 0x9A: // QUERY DEVICE TYPE
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_DEVICE_TYPE, IS_QUERY);
            break;

        case 0x9B: // QUERY PHYISICAL MIN LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_PHYSICAL_MIN_LVL, IS_QUERY);
            break;

        case 0x9C: // QUERY POWER FAILURE
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_POWER_FAILURE, IS_QUERY);
            break;

        case 0x9D: // QUERY CONTENT DTR1
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_CONTENT_DTR_1, IS_QUERY);
            break;

        case 0x9E: // QUERY CONTENT DTR2
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_CONTENT_DTR_2, IS_QUERY);
            break;

        case 0xA1: // QUERY ACTUAL LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_ACTUAL_LVL, IS_QUERY);
            break;

        case 0xA2: // QUERY MAX LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_MAX_LVL, IS_QUERY);
            break;

        case 0xA3: // QUERY MIN LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_MIN_LVL, IS_QUERY);
            break;

        case 0xA4: // QUERY POWER ON LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_POWER_ON_LVL, IS_QUERY);
            break;

        case 0xA5: // QUERY SYSTEM FAILURE LVL
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SYSTEM_FAILURE_LVL, IS_QUERY);
            break;

        case 0xA6: // QUERY FADE TIME / FADE RATE
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_FADE_TIME_RATE, IS_QUERY);
            break;

        case 0xB1: // QUERY SCENE LVL 0
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_0, IS_QUERY);
            break;

        case 0xB2: // QUERY SCENE LVL 1
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_1, IS_QUERY);
            break;

        case 0xB3: // QUERY SCENE LVL 2
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_2, IS_QUERY);
            break;

        case 0xB4: // QUERY SCENE LVL 3
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_3, IS_QUERY);
            break;

        case 0xB5: // QUERY SCENE LVL 4
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_4, IS_QUERY);
            break;

        case 0xB6: // QUERY SCENE LVL 5
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_5, IS_QUERY);
            break;

        case 0xB7: // QUERY SCENE LVL 6
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_6, IS_QUERY);
            break;

        case 0xB8: // QUERY SCENE LVL 7
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_7, IS_QUERY);
            break;

        case 0xB9: // QUERY SCENE LVL 8
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_8, IS_QUERY);
            break;

        case 0xBA: // QUERY SCENE LVL 9
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_9, IS_QUERY);
            break;

        case 0xBB: // QUERY SCENE LVL 10
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_10, IS_QUERY);
            break;

        case 0xBC: // QUERY SCENE LVL 11
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_11, IS_QUERY);
            break;

        case 0xBD: // QUERY SCENE LVL 12
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_12, IS_QUERY);
            break;

        case 0xBE: // QUERY SCENE LVL 13
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_13, IS_QUERY);
            break;

        case 0xBF: // QUERY SCENE LVL 14
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_14, IS_QUERY);
            break;

        case 0xC0: // QUERY SCENE LVL 15
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_SCENE_LVL_15, IS_QUERY);
            break;

        case 0xC1: // QUERY GROUPS 0-7
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_GROUPS_0_7, IS_QUERY);
            break;

        case 0xC2: // QUERY GROUPS 8-15
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_GROUPS_8_15, IS_QUERY);
            break;

        case 0xC3: // QUERY RANDOM ADDRESS HIGH
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_RANDOM_ADDRESS_HIGH, IS_QUERY);
            break;

        case 0xC4: // QUERY RANDOM ADDRESS MEDIUM
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_RANDOM_ADDRESS_MEDIUM, IS_QUERY);
            break;

        case 0xC5: // QUERY RANDOM ADDRESS LOW
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, QUERY_RANDOM_ADDRESS_LOW, IS_QUERY);
            break;

        case 0xC6: // READ MEMORY LOCATION
            sendDaliCommand(_uartPort, DALI_NORMAL_TYPE, subnet, daliAddress, READ_MEMORY_LOCATION, IS_QUERY);
            break;

        case 0xE2: // TERMINATE
            sendDaliSpecialCommand(subnet, TERMINATE, IS_NORMAL);
            break;

        case 0xE5: // RANDOMISE
            sendDaliSpecialCommand(subnet, RANDOMISE, IS_TWICE);
            break;

        case 0xE6: // COMPARE
            sendDaliSpecialCommand(subnet, COMPARE, IS_QUERY);
            break;

        case 0xE7: // WITHDRAW
            sendDaliSpecialCommand(subnet, WITHDRAW, IS_NORMAL);
            break;

        case 0xEF: // QUERY SHORT ADDRESS
            sendDaliSpecialCommand(subnet, QUERY_SHORT_ADDRESS, IS_QUERY);
            break;

        default:
            break;
        }
        break;

    case 0x64:
        switch (commandLow) {
        case 0x00: // REST
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, REST, IS_TWICE);
            break;

        case 0x01: // INHIBIT
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, INHIBIT, IS_TWICE);
            break;

        case 0x02: // RE-LIGHT / RESET INHIBIT
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, RELIGHT_RESET_INHIBIT, IS_TWICE);
            break;

        case 0x03: // START FUNCTION TEST
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, START_FUNCTION_TEST, IS_TWICE);
            break;

        case 0x04: // START DURATION TEST
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, START_DURATION_TEST, IS_TWICE);
            break;

        case 0x05: // STOP TEST
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, STOP_TEST, IS_TWICE);
            break;

        case 0x06: // RESET FUNCTION TEST DONE FLAG
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, RESET_FUNCTION_TEST_FLAG, IS_TWICE);
            break;

        case 0x07: // RESET DURATION TEST DONE FLAG
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, RESET_DURATION_TEST_FLAG, IS_TWICE);
            break;

        case 0x08: // RESET LAMP TIMER
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, RESET_LAMP_TIMER, IS_TWICE);
            break;

        case 0x09: // STORE DTR AS EMERGENCY LVL
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, REST, IS_TWICE);
            break;

        case 0x0A: // STORE TEST DELAY HIGH BYTE
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, STORE_TEST_DELAY_HIGH, IS_TWICE);
            break;

        case 0x0B: // STORE TEST DELAY LOW BYTE
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, STORE_TEST_DELAY_LOW, IS_TWICE);
            break;

        case 0x0C: // STORE FUNCTION TEST INTERVAL
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, STORE_FUNCTION_TEST_INTERVAL, IS_TWICE);
            break;

        case 0x0D: // STORE DURATION TEST INTERVAL
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, STORE_DURATION_TEST_INTERVAL, IS_TWICE);
            break;

        case 0x0E: // STORE TEST EXECUTION TIMEOUT
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, STORE_TEST_EXECUTION_TIMEOUT, IS_TWICE);
            break;

        case 0x0F: // STORE PROLONG TIME
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, STORE_PROLONG_TIME, IS_TWICE);
            break;

        case 0x10: // START IDENTIFICATION
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, START_IDENTIFICATION, IS_NORMAL);
            break;

        case 0x11: // QUERY BATTERY CHARGE
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_BATTERY_CHARGE, IS_QUERY);
            break;

        case 0x12: // QUERY TEST TIMING
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_TEST_TIMING, IS_QUERY);
            break;

        case 0x13: // QUERY DURATION TEST RESULT
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_DURATION_TEST_RESULT, IS_QUERY);
            break;

        case 0x14: // QUERY LAMP EMERGENCY TIME
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_LAMP_EMERGENCY_TIME, IS_QUERY);
            break;

        case 0x15: // QUERY LAMP OPERATION TIME
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_LAMP_OPERATION_TIME, IS_QUERY);
            break;

        case 0x16: // QUERY EMERGENCY TIME
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_LAMP_EMERGENCY_TIME, IS_QUERY);
            break;

        case 0x17: // QUERY EMERGENCY MIN LVL
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_EMERGENCY_MIN_LVL, IS_QUERY);
            break;

        case 0x18: // QUERY EMERGENCY MAX LVL
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_EMERGENCY_MAX_LVL, IS_QUERY);
            break;

        case 0x19: // QUERY RATED DURATION
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_RATED_DURATION, IS_QUERY);
            break;

        case 0x1A: // QUERY EMERGENCY MODE
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_EMERGENCY_MODE, IS_QUERY);
            break;

        case 0x1B: // QUERY FEATURES
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_EMERGENCY_FEATURES, IS_QUERY);
            break;

        case 0x1C: // QUERY FAILURE STATUS
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_FAILURE_STATUS, IS_QUERY);
            break;

        case 0x1D: // QUERY EMERGENCY STATUS
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_EMERGENCY_STATUS, IS_QUERY);
            break;

        case 0x1E: // PERFORM DTR SELECTED FUNCTION
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, PERFORM_DTR_SELECTED_FUNCTION, IS_TWICE);
            break;

        case 0x1F: // QUERY EXTENDED VERSION NUMBER
            sendDaliCommand(_uartPort, DALI_EXTENDED_TYPE, subnet, daliAddress, QUERY_EXTENDED_VERSION_NUMBER, IS_QUERY);
            break;

        default:
            break;
        }
        break;

    case 0xE4:
        switch (commandLow) {
        case 0x00: // REBOOT
            sendDaliCommand(_uartPort, DALI_NORMAGROUP_TYPE, subnet, daliAddress, REBOOT, IS_NORMAL);
            break;

        case 0x20: // BLINK
            sendDaliCommand(_uartPort, DALI_NORMAGROUP_TYPE, subnet, daliAddress, BLINK, IS_NORMAL);
            break;

        case 0x21: // BLINK TERMINATE
            sendDaliCommand(_uartPort, DALI_NORMAGROUP_TYPE, subnet, daliAddress, BLINK_TERMINATE, IS_NORMAL);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

static void processEthFrameType3(QString rcvAddress, QByteArray data, UdpSocket* _udpSocket)
{
    uint8_t commandHigh = (unsigned char)data[8];
    uint8_t commandLow = (unsigned char)data[9];

    uint8_t subnet = (unsigned char)data[4];

    switch (commandHigh) {
    case 0x63:
        switch (commandLow) {
        case 0x00: // ARC POWER
            sendDaliSpecialCommand(subnet, ARC_POWER_DAPC, IS_NORMAL);
            break;

        case 0xE3: // DTR0
            sendDaliSpecialCommand(subnet, DTR_0, IS_NORMAL);
            break;

        case 0xE4: // INITIALISE
            sendDaliSpecialCommand(subnet, INITIALISE, IS_TWICE);
            break;

        case 0xEA: // SEARCH ADDRESS HIGH
            sendDaliSpecialCommand(subnet, SEARCH_ADDRESS_HIGH, IS_NORMAL);
            break;

        case 0xEB: // SEARCH ADDRESS MEDIUM
            sendDaliSpecialCommand(subnet, SEARCH_ADDRESS_MEDIUM, IS_NORMAL);
            break;

        case 0xEC: // SEARCH ADDRESS LOW
            sendDaliSpecialCommand(subnet, SEARCH_ADDRESS_LOW, IS_NORMAL);
            break;

        case 0xED: // PROGRAM SHORT ADDRESS
            sendDaliSpecialCommand(subnet, PROGRAM_SHORT_ADDRESS, IS_NORMAL);
            break;

        case 0xEE: // VERIFY SHORT ADDRESS
            sendDaliSpecialCommand(subnet, VERIFY_SHORT_ADDRESS, IS_QUERY);
            break;

        case 0xF1: // ENABLE DEVICE TYPE
            sendDaliSpecialCommand(subnet, ENABLE_DEVICE_TYPE, IS_NORMAL);
            break;

        case 0xF2: // DTR1
            sendDaliSpecialCommand(subnet, DTR_1, IS_NORMAL);
            break;

        case 0xF3: // DTR2
            sendDaliSpecialCommand(subnet, DTR_2, IS_NORMAL);
            break;

        case 0xF4: // WRITE MEMORY LOCATION
            sendDaliSpecialCommand(subnet, WRITE_MEMORY_LOCATION, IS_QUERY);
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

static void processEthFrameType4(QString rcvAddress, QByteArray data, UdpSocket* _udpSocket)
{
    uint8_t commandHigh = (unsigned char)data[7];
    uint8_t commandLow = (unsigned char)data[8];

    switch (commandHigh) {
    case 0x00:
        switch (commandLow) {
        case 0x01: // SET IP ADDRESS
            setIPAddress(data);
            sendAckFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x03: // SET SUBMASK ADDRESS
            setSubmaskAddress(data);
            sendAckFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x05: // SET GATEWAY ADDRESS
            setGatewayAddress(data);
            sendAckFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x11: // SET BUILDING NAME
            setBuildingName(data);
            sendAckFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x21: // SET LINE NAME
            setLineName(data);
            sendAckFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x31: // SET RTC DATE DAY
            setRtcDateDay(data);
            sendAckFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x33:  // SET RTC DATE SECONDS
            setRtcDateTime(data);
            sendAckFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x51: // SET ADMIN PASSWORD
            setAdminPassword(data);
            sendAckFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x55: // SET MANTENEDOR PASSWORD
            setMantenedorPassword(data);
            sendAckFrame(rcvAddress, commandHigh, commandLow, _udpSocket);
            break;

        case 0x84: // READ HISTORIC READABLE EVENTS
            // ???????
            break;

        case 0x46: // GET MODEL VERSION
            // ???????
            break;

        default:
            break;
        }
        break;

    case 0x40:
        switch (commandLow) {
        case 0x00: // LINE SCANNING START
            // ???????
            break;

        case 0x08: // NEW INITIALISATION START
            // ???????
            break;

        case 0x09: // SYSTEM EXTENSION
            // ???????
            break;

        case 0x18: // READ LINE GROUPS START
            // ???????
            break;

        case 0x19: // READ GROUPS
            // ???????
            break;

        case 0x1A: // WRITE GROUPS
            // ???????
            break;

        case 0x1B: // SAVE CONFIG IN MEMORY
            // ???????
            break;

        case 0x64: // READ TIMERS 0-16
            // ???????
            break;

        case 0x65: // WRITE TIMERS 0-16
            // ???????
            break;

        case 0x84: // READ TEST 0-16
            // ???????
            break;

        case 0x58: // WRITE TEST 0-16
            // ???????
            break;

        default:
            break;
        }
        break;

    case 0x65:
        switch (commandLow) {
        case 0xE7: // GO TO COLOR AND LVL
            // ???????
            break;

        case 0xE8: // GO TO COLOR AND LVL GROUPS
            // ???????
            break;

        case 0xE9: // STORE COLOR SCENE / COLOR SCENES
            // ???????
            break;

        case 0xD1: // CLEAR CIRCADIAN  INHIBIT
            // ???????
            break;

        case 0xD8: // TC STEP COLDER
            // ???????
            break;

        case 0xD9: // TC STEP WARM
            // ???????
            break;

        default:
            break;
        }
        break;

    case 0x77:
        switch (commandLow) {
        case 0x00: // INTERFACE NOTIFICATION
            // ???????
            break;

        case 0x01: // LOOK FOR INTERFACES
            // ???????
            break;

        case 0x02: // SCAN FOR INTERFACES
            // ???????
            break;

        case 0x03: // READ INTERFACES
            // ???????
            break;

        case 0x04: // READ SENSOR ACTIONS
            // ???????
            break;

        case 0x05: // READ SENSOR GROUPS
            // ???????
            break;

        case 0x06: // INIT SENSOR CONFIGURATION
            // ???????
            break;

        case 0x08: // CONFIG INTERFACE N
            // ???????
            break;

        case 0x09: // CONFIG ACTION N
            // ???????
            break;

        case 0x0A: // CONFIG GROUP N
            // ???????
            break;

        case 0x0B: // FINISH CONFIGURATION
            // ???????
            break;

        case 0x88: // -----
            // ???????
            break;

        case 0xBA: // SENSOR SET POINT
            // ???????
            break;

        case 0xBB: // SENSOR GET POINT
            // ???????
            break;

        case 0xC0: // CMD DALI
            // ???????
            break;

        default:
            break;
        }
        break;

    case 0x78:
        switch (commandLow) {
        case 0x01: // MASTER PING
            // ???????
            break;

        default:
            break;
        }
        break;

    default:
        break;
    }
}

void processEthFrame(QString rcvAddress, QByteArray data, UdpSocket* _udpSocket, UartPort* _uartPort)
{
    uint8_t frameType = (unsigned char)data[3];

    switch(frameType) {
    case 0x00:
        processEthFrameType0(rcvAddress, data, _udpSocket);
        break;

    case 0x01:
        processEthFrameType1(rcvAddress, data, _udpSocket, _uartPort);
        break;

    case 0x03:
        processEthFrameType3(rcvAddress, data, _udpSocket);
        break;

    case 0x04:
        processEthFrameType4(rcvAddress, data, _udpSocket);
        break;

    default:
        break;
    }
}

void sendAckFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_82);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x01);
    frame.append((unsigned char)0x00);

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}
