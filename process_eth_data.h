#ifndef PROCESS_ETH_DATA_H
#define PROCESS_ETH_DATA_H

#include "UdpSocket.h"
#include "UartPort.h"

bool checkCRC(QByteArray data);
bool checkFrameHeader(QByteArray data);
bool checkRcvAddress(QString rcvAddress);

void processEthFrame(QString rcvAddress, QByteArray data, UdpSocket* _udpSocket, UartPort* _uartPort);

void sendAckFrame(QString rcvAddress, uint8_t commandHigh, uint8_t commandLow, UdpSocket* _udpSocket);

#endif // PROCESS_ETH_DATA_H
