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

void writeIMU_frame(int fd, const std::vector<uint8_t>& data) {
    ssize_t bytes_written = write(fd, data.data(), data.size());  // write to file descriptor pointing the serial port
    if (bytes_written < 0) {
        throw std::runtime_error("Error writing to UART port");
    }
}

/*
 * Argument: File path of a port, ex: /dev/pts/2
 */
int main(int argc, char* argv[]) {
    int fd = open((argc > 1) ? argv[1] : UART_PORT_FILE, O_RDWR | O_NOCTTY | O_NONBLOCK);  // open port provided as argument
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

