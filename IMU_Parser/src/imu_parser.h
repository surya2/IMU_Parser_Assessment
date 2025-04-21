//
// Created by Surya Selvam on 4/19/2025.
//

#ifndef TRUEANOMALYASSESSMENT_IMU_PARSER_H
#define TRUEANOMALYASSESSMENT_IMU_PARSER_H

#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <vector>
#include <csignal>
#include <atomic>
#include <memory>
#include <iomanip>

#include "parse_functions.h"

#ifdef __linux__
#include <termios.h>
#elif _WIN32 || _WIN64
#include <conio.h>
#endif

#define UART_PORT_FILE "/dev/tty1"


#define BROADCAST_PORT 8888
#define LOCALHOST_BROADCAST_IP "127.255.255.255"

void processIMU_Frames();

#endif //TRUEANOMALYASSESSMENT_IMU_PARSER_H
