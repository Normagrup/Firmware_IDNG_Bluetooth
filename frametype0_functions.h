#ifndef FRAMETYPE0_FUNCTIONS_H
#define FRAMETYPE0_FUNCTIONS_H

#include <QObject>

#include "UdpSocket.h"
#include "eth_frames.h"

void sendIPAddressFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendSubmaskAddressFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendGatewayAddressFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendMacAddressFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendBuildingNameFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendLineNameFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendRtcDateDayFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendRtcDateDaySeconds(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendModelVersionFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendFirmwareVersionFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendAdminPasswordFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendMantenedorPasswordFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);

void sendPingFrame(QString rcvAddress, uint8_t packetID, UdpSocket* _udpSocket);

void sendDaliLineInstalledDevices(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);
void sendDaliLineReadDevicesType(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket);
void sendDaliLineReadComunicationState(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket);
void sendDaliLineQueryActualLvl(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket);
void sendDaliLineQueryStatus(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket);
void sendDaliLineQueryEmergencyMode(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket);
void sendDaliLineQueryFailureStatus(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket *_udpSocket);

#endif // FRAMETYPE0_FUNCTIONS_H
