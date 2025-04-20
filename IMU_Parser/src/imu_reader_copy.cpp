//
// Created by Surya Selvam on 4/19/2025.
//

#include <iostream>
#include <fcntl.h>
#include <string.h>
#include <mutex>
#include <thread>
#include <vector>
#include <csignal>
#include <atomic>
#include <memory>

#include "imu_reader.h"
#include "compatibility.h"

std::vector<char> dataBuffer;
struct sockaddr_in broadcastAddress;
bool little_endian = false;
int broadcastSocket = -1;
int port_fd = -1;

int findHeader(uint8_t *position){
    std::cout << "data buffer size: " << static_cast<int>(dataBuffer.size())-(int)HEADER_SIZE << std::endl;
    for(int i = 0; i<static_cast<int>(dataBuffer.size())-(int)HEADER_SIZE; i++){
        std::cout << i << std::endl;
        if(dataBuffer[i] == HEADER1 && dataBuffer[i+1] == HEADER2 && dataBuffer[i+2] == HEADER3 && dataBuffer[i+3] == HEADER4){
            *position = i;
            return true;
        }
    }
    return false;
}

void processIMU_Frames(){
    dataBuffer.reserve(1024);
    uint8_t position = 0;
    while(true){
        uint8_t stagingBuffer[128];
        memset(&stagingBuffer, 0, sizeof(stagingBuffer));
        int nBytes = read(port_fd, stagingBuffer, sizeof(stagingBuffer));
        if(nBytes > 0){
            std::cout << "Got something" << std::endl;
            dataBuffer.insert(dataBuffer.end(), stagingBuffer, stagingBuffer+nBytes);
            if(findHeader(&position)) {
                if ((position + PACKET_SIZE) <= dataBuffer.size()){
                    ParsedPacket packet;
                    memcpy(&packet.packetCount, &dataBuffer[position + 4], 4);
                    memcpy(&packet.X_GyroRate, &dataBuffer[position + 8], 4);
                    memcpy(&packet.Y_GyroRate, &dataBuffer[position + 12], 4);
                    memcpy(&packet.Z_GyroRate, &dataBuffer[position + 16], 4);

                    sendto(broadcastSocket, (char *) &packet, sizeof(packet), 0,
                           (struct sockaddr *) &broadcastAddress, sizeof(broadcastAddress));

                    std::cout << "Packet Count: " << packet.packetCount
                              << ", X Gyro: " << packet.X_GyroRate
                              << ", Y Gyro: " << packet.Y_GyroRate
                              << ", Z Gyro: " << packet.Z_GyroRate << std::endl;
                    dataBuffer.clear();
                    position = 0;
                }
                break;
            }
        } else if (nBytes < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            } else {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
            }
        }
    }
}

int main() {
    int test_number = 1;
    char *test_number_ptr = (char*)&test_number;

    if (*test_number_ptr == 1) {
        little_endian = true;
    }

    port_fd = open(UART_PORT_FILE, O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (port_fd == -1) {
        throw std::runtime_error("UART port could no be opened...");
        return -1;
    } else {
        std::cout << "just opened the port at " << port_fd << std::endl;
    }

    struct termios tty_config;
    memset(&tty_config, 0, sizeof(tty_config));
    if (tcgetattr(port_fd, &tty_config) != 0) {
        close(port_fd);
        throw std::runtime_error("Could not retrieve UART port configuraions");
        return -1;
    }

    /* Set receiving baud rate to same as IMU transmitter */
    cfsetospeed(&tty_config, B921600);
    cfsetispeed(&tty_config, B921600);

    // After getting attributes
    tty_config.c_cflag &= ~PARENB;  // No parity
    tty_config.c_cflag &= ~CSTOPB;  // 1 stop bit
    tty_config.c_cflag &= ~CSIZE;
    tty_config.c_cflag |= CS8;      // 8 data bits
    tty_config.c_cflag &= ~CRTSCTS; // No flow control
    tty_config.c_iflag &= ~(IXON | IXOFF | IXANY); // No software flow control
    tty_config.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
    tty_config.c_oflag &= ~OPOST;  // Raw output

    if (tcsetattr(port_fd, TCSANOW, &tty_config) != 0) {
        close(port_fd);
        throw std::runtime_error("Could not apply UART port configurations");
        return -1;
    }

    broadcastSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (broadcastSocket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        exit(1);
    }
    int broadcastEnable = 1;
    if (setsockopt(broadcastSocket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0) {
        std::cerr << "Failed to set socket options" << std::endl;
        close(broadcastSocket);
        exit(1);
    }

    memset(&broadcastAddress, 0, sizeof(broadcastAddress));
    broadcastAddress.sin_family = AF_INET;
    broadcastAddress.sin_port = htons(BROADCAST_PORT);
    inet_pton(AF_INET, LOCALHOST_BROADCAST_IP, &broadcastAddress.sin_addr);

    while(true) {
        processIMU_Frames();
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
    }

    close(port_fd);
    return 0;
}




