//
// Created by Surya Selvam on 4/19/2025.
//

#ifndef TRUEANOMALYASSESSMENT_COMPATIBILITY_H
#define TRUEANOMALYASSESSMENT_COMPATIBILITY_H

#ifdef _WIN32
void usleep_simulation(unsigned int microseconds);
#define usleep usleep_simulation
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
#define close closesocket
#else
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#endif //TRUEANOMALYASSESSMENT_COMPATIBILITY_H
