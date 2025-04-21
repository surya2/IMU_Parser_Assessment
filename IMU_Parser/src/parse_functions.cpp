//
// Created by Surya Selvam on 4/21/2025.
//

#include "parse_functions.h"
#include "compatibility.h"


int findHeader(std::vector<uint8_t>& dataBuffer, uint8_t *position){
    if(dataBuffer.size() < HEADER_SIZE){
        return false;
    }
    for(int i = 0; i<static_cast<int>(dataBuffer.size())-(int)HEADER_SIZE; i++){  // loop through inputBuffer
        if(dataBuffer[i] == HEADER1 && dataBuffer[i+1] == HEADER2 && dataBuffer[i+2] == HEADER3 && dataBuffer[i+3] == HEADER4){   // if headers found, return true
            *position = i;
            return true;
        }
    }
    return false;
}

std::pair<ParsedPacket, bool> parsePacket(std::vector<uint8_t>& dataBuffer, uint8_t *position, bool little_endian){
    ParsedPacket packet{};
    if(findHeader(dataBuffer, position)) {
        if ((*position + PACKET_SIZE) <= dataBuffer.size()){  // if headers with packet are found (full packet size can be extracted)
            // Copy data corresponding to each field of Packet count, X-axis Gyro Rate, Y-axis Gyro Rate, Z-axis Gyro Rate into ParsedPacket struct
            memcpy(&packet.packetCount, &dataBuffer[*position + 4], 4);
            memcpy(&packet.X_GyroRate, &dataBuffer[*position + 8], 4);
            memcpy(&packet.Y_GyroRate, &dataBuffer[*position + 12], 4);
            memcpy(&packet.Z_GyroRate, &dataBuffer[*position + 16], 4);

            // If little-endian machine, use ntohl() to convert into host machine's endiannes. I also provided some code for manually doing this conversion in my README.ms
            if (little_endian) {
                packet.packetCount = ntohl(packet.packetCount);
                uint32_t temp;
                memcpy(&temp, &packet.X_GyroRate, 4);
                temp = ntohl(temp);
                memcpy(&packet.X_GyroRate, &temp, 4);

                memcpy(&temp, &packet.Y_GyroRate, 4);
                temp = ntohl(temp);
                memcpy(&packet.Y_GyroRate, &temp, 4);

                memcpy(&temp, &packet.Z_GyroRate, 4);
                temp = ntohl(temp);
                memcpy(&packet.Z_GyroRate, &temp, 4);
            }

            dataBuffer.clear();  // clear because full packet has been found, now wait 80ms until the buffer is used again to recreate packets from the UART frames received
            *position = 0;
            return {packet, true};
        }
    }
    return {packet, false};
}
