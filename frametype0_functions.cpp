#include "frametype0_functions.h"
#include "global_variables.h"
#include "version_config.h"
#include "file_handler.h"
#include "Pollings.h"

void sendIPAddressFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x03);
    frame.append(networkIP[0].toUInt());
    frame.append(networkIP[1].toUInt());
    frame.append(networkIP[2].toUInt());
    frame.append(networkIP[3].toUInt());

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendSubmaskAddressFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x03);
    frame.append(networkSubmask[0].toUInt());
    frame.append(networkSubmask[1].toUInt());
    frame.append(networkSubmask[2].toUInt());
    frame.append(networkSubmask[3].toUInt());

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendGatewayAddressFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x03);
    frame.append(gatewayAddress[0].toUInt());
    frame.append(gatewayAddress[1].toUInt());
    frame.append(gatewayAddress[2].toUInt());
    frame.append(gatewayAddress[3].toUInt());

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendMacAddressFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x05);
    frame.append(QByteArray::fromHex(macAddress[0].toLatin1()));
    frame.append(QByteArray::fromHex(macAddress[1].toLatin1()));
    frame.append(QByteArray::fromHex(macAddress[2].toLatin1()));
    frame.append(QByteArray::fromHex(macAddress[3].toLatin1()));
    frame.append(QByteArray::fromHex(macAddress[4].toLatin1()));
    frame.append(QByteArray::fromHex(macAddress[5].toLatin1()));

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendBuildingNameFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;
    uint8_t data[16];

    getBuildingName(&data[0]);

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x0F);
    frame.append(data[0]);
    frame.append(data[1]);
    frame.append(data[2]);
    frame.append(data[3]);
    frame.append(data[4]);
    frame.append(data[5]);
    frame.append(data[6]);
    frame.append(data[7]);
    frame.append(data[8]);
    frame.append(data[9]);
    frame.append(data[10]);
    frame.append(data[11]);
    frame.append(data[12]);
    frame.append(data[13]);
    frame.append(data[14]);
    frame.append(data[15]);

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendLineNameFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;
    uint8_t data[16];

    getLineName(&data[0]);

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x0F);
    frame.append(data[0]);
    frame.append(data[1]);
    frame.append(data[2]);
    frame.append(data[3]);
    frame.append(data[4]);
    frame.append(data[5]);
    frame.append(data[6]);
    frame.append(data[7]);
    frame.append(data[8]);
    frame.append(data[9]);
    frame.append(data[10]);
    frame.append(data[11]);
    frame.append(data[12]);
    frame.append(data[13]);
    frame.append(data[14]);
    frame.append(data[15]);

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendRtcDateDayFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;
    uint8_t data[4];

    getRtcDate(&data[0]);

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x03);
    frame.append(data[3]);
    frame.append(data[2]);
    frame.append(data[1]);
    frame.append(data[0]);

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendRtcDateDaySeconds(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;
    uint8_t data[3];

    getRtcSeconds(&data[0]);

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x02);
    frame.append(data[2]);
    frame.append(data[1]);
    frame.append(data[0]);

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendModelVersionFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x01);
    frame.append(MODEL_H);
    frame.append(MODEL_L);

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendFirmwareVersionFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x01);
    frame.append(FIRMWARE_VERSION_H);
    frame.append(FIRMWARE_VERSION_L);

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendAdminPasswordFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;
    uint8_t data[16];

    getAdminPassword(&data[0]);

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x0F);
    frame.append(data[0]);
    frame.append(data[1]);
    frame.append(data[2]);
    frame.append(data[3]);
    frame.append(data[4]);
    frame.append(data[5]);
    frame.append(data[6]);
    frame.append(data[7]);
    frame.append(data[8]);
    frame.append(data[9]);
    frame.append(data[10]);
    frame.append(data[11]);
    frame.append(data[12]);
    frame.append(data[13]);
    frame.append(data[14]);
    frame.append(data[15]);

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendMantenedorPasswordFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;
    uint8_t data[16];

    getMantenedorPassword(&data[0]);

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_83);
    frame.append(commandHigh);
    frame.append(commandLow);
    frame.append(0x0F);
    frame.append(data[0]);
    frame.append(data[1]);
    frame.append(data[2]);
    frame.append(data[3]);
    frame.append(data[4]);
    frame.append(data[5]);
    frame.append(data[6]);
    frame.append(data[7]);
    frame.append(data[8]);
    frame.append(data[9]);
    frame.append(data[10]);
    frame.append(data[11]);
    frame.append(data[12]);
    frame.append(data[13]);
    frame.append(data[14]);
    frame.append(data[15]);

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendPingFrame(QString rcvAddress, uint8_t packetID, UdpSocket* _udpSocket)
{
    QByteArray frame;
    unsigned char crc = 0;

    frame.append(FRAME_HEADER_0);
    frame.append(FRAME_HEADER_1);
    frame.append(FRAME_HEADER_2);
    frame.append(FRAME_TYPE_82);
    frame.append(packetID);
    frame.append(RANDOM_BYTE);
    frame.append(0x01);
    frame.append(DEVICE_ID);

    for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

    frame.append(crc);

    QHostAddress dstAddress;
    dstAddress.setAddress(rcvAddress);

    _udpSocket->sendData(dstAddress, frame);
}

void sendDaliLineInstalledDevices(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket)
{
    for (uint8_t subnet = SUBNET_OFFSET; subnet  <  MAX_SUBNET; subnet++) {
        if (polling.isSubnetConfigured(subnet - SUBNET_OFFSET)) {
            QByteArray frame;
            unsigned char crc = 0;

            frame.append(FRAME_HEADER_0);
            frame.append(FRAME_HEADER_1);
            frame.append(FRAME_HEADER_2);
            frame.append(FRAME_TYPE_90);
            frame.append(subnet - SUBNET_OFFSET);
            frame.append(commandHigh);
            frame.append(commandLow);
            frame.append(polling.getArrayConfiguredDevices(subnet));

            for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

            frame.append(crc);

            QHostAddress dstAddress;
            dstAddress.setAddress(rcvAddress);

            _udpSocket->sendData(dstAddress, frame);
        }
    }
}

void sendDaliLineReadDevicesType(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket)
{
    for (uint8_t subnet = SUBNET_OFFSET; subnet  <  MAX_SUBNET; subnet++) {
        if (polling.isSubnetConfigured(subnet - SUBNET_OFFSET)) {
            QByteArray frame;
            unsigned char crc = 0;

            frame.append(FRAME_HEADER_0);
            frame.append(FRAME_HEADER_1);
            frame.append(FRAME_HEADER_2);
            frame.append(FRAME_TYPE_90);
            frame.append(subnet - SUBNET_OFFSET);
            frame.append(commandHigh);
            frame.append(commandLow);
            frame.append(polling.getArrayDevicesType(subnet));

            for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

            frame.append(crc);

            QHostAddress dstAddress;
            dstAddress.setAddress(rcvAddress);

            _udpSocket->sendData(dstAddress, frame);
        }
    }
}

void sendDaliLineReadComunicationState(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket)
{
    for (uint8_t subnet = SUBNET_OFFSET; subnet  <  MAX_SUBNET; subnet++) {
        if (polling.isSubnetConfigured(subnet - SUBNET_OFFSET)) {
            QByteArray frame;
            unsigned char crc = 0;

            frame.append(FRAME_HEADER_0);
            frame.append(FRAME_HEADER_1);
            frame.append(FRAME_HEADER_2);
            frame.append(FRAME_TYPE_90);
            frame.append(subnet - SUBNET_OFFSET);
            frame.append(commandHigh);
            frame.append(commandLow);
            frame.append(polling.getArrayComunicationFailure(subnet));

            for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

            frame.append(crc);

            QHostAddress dstAddress;
            dstAddress.setAddress(rcvAddress);

            _udpSocket->sendData(dstAddress, frame);
        }
    }
}

void sendDaliLineQueryActualLvl(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket)
{
    for (uint8_t subnet = SUBNET_OFFSET; subnet  <  MAX_SUBNET; subnet++) {
        if (polling.isSubnetConfigured(subnet - SUBNET_OFFSET)) {
            QByteArray frame;
            unsigned char crc = 0;

            frame.append(FRAME_HEADER_0);
            frame.append(FRAME_HEADER_1);
            frame.append(FRAME_HEADER_2);
            frame.append(FRAME_TYPE_90);
            frame.append(subnet - SUBNET_OFFSET);
            frame.append(commandHigh);
            frame.append(commandLow);
            frame.append(polling.getArrayQueryActualLvl(subnet));

            for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

            frame.append(crc);

            QHostAddress dstAddress;
            dstAddress.setAddress(rcvAddress);

            _udpSocket->sendData(dstAddress, frame);
        }
    }
}

void sendDaliLineQueryStatus(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket)
{
    for (uint8_t subnet = SUBNET_OFFSET; subnet  <  MAX_SUBNET; subnet++) {
        if (polling.isSubnetConfigured(subnet - SUBNET_OFFSET)) {
            QByteArray frame;
            unsigned char crc = 0;

            frame.append(FRAME_HEADER_0);
            frame.append(FRAME_HEADER_1);
            frame.append(FRAME_HEADER_2);
            frame.append(FRAME_TYPE_90);
            frame.append(subnet - SUBNET_OFFSET);
            frame.append(commandHigh);
            frame.append(commandLow);
            frame.append(polling.getArrayQueryStatus(subnet));

            for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

            frame.append(crc);

            QHostAddress dstAddress;
            dstAddress.setAddress(rcvAddress);

            _udpSocket->sendData(dstAddress, frame);
        }
    }
}

void sendDaliLineQueryEmergencyMode(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket)
{
    for (uint8_t subnet = SUBNET_OFFSET; subnet  <  MAX_SUBNET; subnet++) {
        if (polling.isSubnetConfigured(subnet - SUBNET_OFFSET)) {
            QByteArray frame;
            unsigned char crc = 0;

            frame.append(FRAME_HEADER_0);
            frame.append(FRAME_HEADER_1);
            frame.append(FRAME_HEADER_2);
            frame.append(FRAME_TYPE_90);
            frame.append(subnet - SUBNET_OFFSET);
            frame.append(commandHigh);
            frame.append(commandLow);
            frame.append(polling.getArrayQueryEmergencyMode(subnet));

            for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

            frame.append(crc);

            QHostAddress dstAddress;
            dstAddress.setAddress(rcvAddress);

            _udpSocket->sendData(dstAddress, frame);
        }
    }
}

void sendDaliLineQueryFailureStatus(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket)
{
    for (uint8_t subnet = SUBNET_OFFSET; subnet  <  MAX_SUBNET; subnet++) {
        if (polling.isSubnetConfigured(subnet - SUBNET_OFFSET)) {
            QByteArray frame;
            unsigned char crc = 0;

            frame.append(FRAME_HEADER_0);
            frame.append(FRAME_HEADER_1);
            frame.append(FRAME_HEADER_2);
            frame.append(FRAME_TYPE_90);
            frame.append(subnet - SUBNET_OFFSET);
            frame.append(commandHigh);
            frame.append(commandLow);
            frame.append(polling.getArrayQueryFailureStatus(subnet));

            for (uint8_t i = 3; i < frame.size(); i++) { crc += frame[i]; }

            frame.append(crc);

            QHostAddress dstAddress;
            dstAddress.setAddress(rcvAddress);

            _udpSocket->sendData(dstAddress, frame);
        }
    }
}
