//
// Created by Surya Selvam on 4/19/2025.
//

#ifndef TRUEANOMALYASSESSMENT_IMU_READER_H
#define TRUEANOMALYASSESSMENT_IMU_READER_H

#ifdef __linux__ // for POSIX OS
#include <termios.h>
#elif _WIN32 || _WIN64 // for Windows
#include <conio.h>
#endif

#include <unistd.h>

#define UART_PORT_FILE "/dev/tty1"
#define PACKET_SIZE 20
#define HEADER_SIZE 4

#define HEADER1 0x7F
#define HEADER2 0xF0
#define HEADER3 0x1C
#define HEADER4 0xAF

#define BROADCAST_PORT 8888
#define LOCALHOST_BROADCAST_IP "127.255.255.255"

struct ParsedPacket {
    int packetCount;
    float X_GyroRate;
    float Y_GyroRate;
    float Z_GyroRate;
};

#endif //TRUEANOMALYASSESSMENT_IMU_READER_H
