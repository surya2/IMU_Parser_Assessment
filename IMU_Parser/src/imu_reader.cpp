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
std::mutex dataBufferMtx;
struct sockaddr_in broadcastAddress;
bool little_endian = false;
int broadcastSocket = -1;
int port_fd = -1;
std::shared_ptr<std::atomic<bool>> halt = std::make_shared<std::atomic<bool>>(false);

void interrupt_handler(int signal_num){
    std::cerr << "Signal interrupt raised with exit code " << signal_num << std::endl;
    halt->store(true);
}

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

ParsedPacket parse(std::vector<char>& buffer, std::mutex& mtx){
    mtx.lock();
    uint8_t position = 0;
    ParsedPacket packet;
    if(findHeader(&position) && (position + PACKET_SIZE) <= buffer.size()){
        memcpy(&packet.packetCount, &buffer[position+4], 4);
        memcpy(&packet.X_GyroRate, &buffer[position+8], 4);
        memcpy(&packet.Y_GyroRate, &buffer[position+12], 4);
        memcpy(&packet.Z_GyroRate, &buffer[position+16], 4);
        std::cout << "Packet Count: " << packet.packetCount << ", X Gyro: " << packet.X_GyroRate << ", Y Gyro: " << packet.Z_GyroRate << "\n" << std::endl;
    }

    if (position > 0) {
        buffer.erase(buffer.begin(), buffer.begin() + position);
    }
    mtx.unlock();
    return packet;
}

void broadcast(std::vector<char>& buffer, std::mutex& mtx){
    while(!halt->load()) {
        auto packet = parse(buffer, mtx);
        std::cout << "Finished parsing" << std::endl;
        std::cout << "size of " << sizeof(packet) << std::endl;
        printf("packet %s", (char *) &packet);
        sendto(broadcastSocket, (char *) &packet, sizeof(packet), 0, (struct sockaddr *) &broadcastAddress,
               sizeof(broadcastAddress));
    }
}

void receive(std::vector<char>& buffer, std::mutex& mtx){
    while(!halt->load()) {
        uint8_t stagingBuffer[128];
        std::cout << "Staging buffer size: " << sizeof(stagingBuffer) << std::endl;
        memset(&stagingBuffer, 0, sizeof(stagingBuffer));
        int nBytes = read(port_fd, stagingBuffer, sizeof(stagingBuffer));
        if(nBytes > 0){
            std::cout << "Got something" << std::endl;
            mtx.lock();
            buffer.insert(buffer.end(), stagingBuffer, stagingBuffer+nBytes);
            mtx.unlock();
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cout << "Blocking" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            } else {
                std::cerr << "Read error: " << strerror(errno) << std::endl;
            }
        }

//        if(dataBuffer.size() > 1024){
//            dataBuffer.erase(dataBuffer.begin(), dataBuffer.begin() + 512);
//        }
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
        std::cout << "just openned the port at " << port_fd << std::endl;
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

// Apply the settings
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

    // threads
    std::vector<std::thread> threads;
    std::cout << "Hello" << std::endl;
    threads.push_back(std::thread(receive, std::ref(dataBuffer), std::ref(dataBufferMtx)));
    threads.push_back(std::thread(broadcast, std::ref(dataBuffer), std::ref(dataBufferMtx)));

    for(auto& th : threads){
        th.join();
    }

    close(port_fd);
    return 0;
}




