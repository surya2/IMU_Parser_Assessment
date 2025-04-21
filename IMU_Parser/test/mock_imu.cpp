//
// Created by Surya Selvam on 4/19/2025.
//

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <stdexcept>
#include "../src/imu_parser.h"
#include <thread>

#include "fake_data.h"

//void configure_uart(int fd) {
//    struct termios tty;
//
//    if (tcgetattr(fd, &tty) != 0) {
//        throw std::runtime_error("Error getting terminal attributes");
//    }
//
//    cfsetispeed(&tty, B921600);  // Input baud rate
//    cfsetospeed(&tty, B921600);  // Output baud rate
//
//    // Set 8N1 mode (8 data bits, no parity, 1 stop bit)
//    tty.c_cflag &= ~PARENB;  // No parity
//    tty.c_cflag &= ~CSTOPB;  // 1 stop bit
//    tty.c_cflag &= ~CSIZE;
//    tty.c_cflag |= CS8;      // 8 data bits
//
//    // Disable hardware flow control
//    tty.c_cflag &= ~CRTSCTS;
//
//    // Disable software flow control
//    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
//
//    // Set the mode to raw (no processing of input/output)
//    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  // Raw input
//    tty.c_oflag &= ~OPOST;  // Raw output
//
//    // Apply the new settings
//    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
//        throw std::runtime_error("Error setting terminal attributes");
//    }
//}

void writeIMU_frame(int fd, const std::vector<uint8_t>& data) {
    ssize_t bytes_written = write(fd, data.data(), data.size());
    if (bytes_written < 0) {
        throw std::runtime_error("Error writing to UART port");
    }
}

int main(int argc, char* argv[]) {
    int fd = open((argc > 1) ? argv[1] : UART_PORT_FILE, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd == -1) {
        std::cerr << "Error opening UART port: " << strerror(errno) << std::endl;
        return -1;
    }

    try {
        struct termios tty;

        if (tcgetattr(fd, &tty) != 0) {
            throw std::runtime_error("Error getting terminal attributes");
        }

        //cfsetispeed(&tty, B921600);
        cfsetospeed(&tty, B921600);  // Setting the output/write baud rate to 921600 as per the requirements

        std::vector<uint8_t> packet = createFakePacket();

        while(true){
            writeIMU_frame(fd, packet);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

        close(fd);
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        close(fd);
        return -1;
    }

    return 0;
}

