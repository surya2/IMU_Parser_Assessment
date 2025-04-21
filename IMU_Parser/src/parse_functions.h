//
// Created by Surya Selvam on 4/21/2025.
//

#ifndef TRUEANOMALYASSESSMENT_PARSE_FUNCTIONS_H
#define TRUEANOMALYASSESSMENT_PARSE_FUNCTIONS_H

#include <unistd.h>
#include <vector>
#include <iostream>
#include <memory>
#include <string.h>

#define PACKET_SIZE 20
#define HEADER_SIZE 4

#define HEADER1 0x7F
#define HEADER2 0xF0
#define HEADER3 0x1C
#define HEADER4 0xAF

struct ParsedPacket {
    int packetCount;
    float X_GyroRate;
    float Y_GyroRate;
    float Z_GyroRate;
};

int findHeader(std::vector<uint8_t>& dataBuffer, uint8_t *position);
ParsedPacket* parsePacket(std::vector<uint8_t>& dataBuffer, uint8_t *position, bool little_endian);

#endif //TRUEANOMALYASSESSMENT_PARSE_FUNCTIONS_H
