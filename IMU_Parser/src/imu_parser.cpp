//
// Created by Surya Selvam on 4/19/2025.
//

#include "imu_parser.h"
#include "compatibility.h"

std::vector<uint8_t> inputBuffer;
struct sockaddr_in broadcastAddress;
bool little_endian = false;
int broadcastSocket = -1;
int port_fd = -1;

void processIMU_Frames(){
    inputBuffer.reserve(1024);
    uint8_t indexInBuffer = 0;  // To keep track of position in inputBuffer to start parsing packet. Sometimes you could get the middle of UART frame and later get the next packet. So it is necessary to increment into the received byte stream to find the mosty recent packet to parse.
    while(true){
        uint8_t stagingBuffer[128];  // For receiving byte stream from UART device
        memset(&stagingBuffer, 0, sizeof(stagingBuffer));
        int nBytes = read(port_fd, stagingBuffer, sizeof(stagingBuffer));
        if(nBytes > 0){
            inputBuffer.insert(inputBuffer.end(), stagingBuffer, stagingBuffer+nBytes);  // Add to inputBuffer; inputBuffer's job is to get all of the bytes so that findHeader() can go in and find a full packet
            auto parseResult = parsePacket(inputBuffer, &indexInBuffer, (bool)little_endian);  // call to parsePacket to get a packet
            if(parseResult.second) {
                // broadcast message to 127.255.255.255 - all devices/processes on localhost network
                sendto(broadcastSocket, (char *) &(parseResult.first), sizeof(parseResult.first), 0,
                   (struct sockaddr *) &broadcastAddress, sizeof(broadcastAddress));
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

int main(int argc, char* argv[]) {
    int test_number = 1;
    char *test_number_ptr = (char*)&test_number;

    if (*test_number_ptr == 1) {
        little_endian = true;
    }

    const char *uart_file = (argc > 1) ? argv[1] : UART_PORT_FILE;

    port_fd = open(uart_file, O_RDONLY | O_NOCTTY | O_NONBLOCK);
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

    cfsetispeed(&tty_config, B921600); // Setting receiver input baud rate to same as the IMU transmitter

    broadcastSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (broadcastSocket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        exit(1);
    }
    int enable = 1;
    if (setsockopt(broadcastSocket, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) < 0) {
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
        std::this_thread::sleep_for(std::chrono::milliseconds(80)); // 80 millisecond wait: Using this_thread allows the sleep to occur as if it is in a separate thread, allowing other processes to occupy the existing core
    }

    close(port_fd);
    return 0;
}




