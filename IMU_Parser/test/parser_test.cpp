//
// Created by Surya Selvam on 4/20/2025.
//

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() function
#include <catch2/catch.hpp>  // Or the appropriate include path for your setup
#include <mutex>
#include <thread>
#include <vector>
#include <arpa/inet.h>
#include <utility>

#include "../src/imu_parser.h"
#include "compatibility.h"
#include "../src/parse_functions.h"

std::vector<uint8_t> dataBuffer;

void addPacketToBuffer(std::vector<uint8_t>& buffer, uint32_t packetCount, float xGyro, float yGyro, float zGyro) {
    buffer.push_back(0x7F);
    buffer.push_back(0xF0);
    buffer.push_back(0x1C);
    buffer.push_back(0xAF);

    uint32_t packetC = htonl(packetCount);
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&packetC);
    for(int i = 0; i < 4; i++) {
        buffer.push_back(bytes[i]);
    }

    uint32_t xGyroBytes;
    memcpy(&xGyroBytes, &xGyro, sizeof(float));
    xGyroBytes = htonl(xGyroBytes);
    bytes = reinterpret_cast<uint8_t*>(&xGyroBytes);
    for(int i = 0; i < 4; i++) {
        buffer.push_back(bytes[i]);
    }

    uint32_t yGyroBytes;
    memcpy(&yGyroBytes, &yGyro, sizeof(float));
    yGyroBytes = htonl(yGyroBytes);
    bytes = reinterpret_cast<uint8_t*>(&yGyroBytes);
    for(int i = 0; i < 4; i++) {
        buffer.push_back(bytes[i]);
    }

    uint32_t zGyroBytes;
    memcpy(&zGyroBytes, &zGyro, sizeof(float));
    zGyroBytes = htonl(zGyroBytes);
    bytes = reinterpret_cast<uint8_t*>(&zGyroBytes);
    for(int i = 0; i < 4; i++) {
        buffer.push_back(bytes[i]);
    }
}

TEST_CASE("Test single packet")
{
    uint8_t position = 0;
    addPacketToBuffer(dataBuffer, 1, 1.0, 1.3, 1.5);
    auto parsedResult = parsePacket(dataBuffer, &position, true);
    REQUIRE(parsedResult.second == true);
    REQUIRE(((ParsedPacket)parsedResult.first).packetCount == 1);
    REQUIRE(((ParsedPacket)parsedResult.first).X_GyroRate == 1.0);
    REQUIRE(((ParsedPacket)parsedResult.first).Y_GyroRate == 1.3f);
    REQUIRE(((ParsedPacket)parsedResult.first).Z_GyroRate == 1.5f);
    dataBuffer.clear();
}

TEST_CASE("Test single malformed header packet")
{
    uint8_t position = 0;
    addPacketToBuffer(dataBuffer, 1, 1.0, 1.3, 1.5);
    dataBuffer[0] = 0x8f;
    auto parsedResult = parsePacket(dataBuffer, &position, true);
    REQUIRE(parsedResult.second == false);
    REQUIRE(((ParsedPacket)parsedResult.first).packetCount == 0);
    REQUIRE(((ParsedPacket)parsedResult.first).X_GyroRate == 0.0);
    REQUIRE(((ParsedPacket)parsedResult.first).Y_GyroRate == 0.0);
    REQUIRE(((ParsedPacket)parsedResult.first).Z_GyroRate == 0.0);
    dataBuffer.clear();
}

TEST_CASE("Test multiple full packets")
{
    uint8_t position = 0;
    addPacketToBuffer(dataBuffer, 2, 6.78, 98.8, 4.8956);
    addPacketToBuffer(dataBuffer, 1, 1.0, 1.3, 1.5);
    auto parsedResult = parsePacket(dataBuffer, &position, true);
    REQUIRE(parsedResult.second == true);
    REQUIRE(((ParsedPacket)parsedResult.first).packetCount == 2);
    REQUIRE(((ParsedPacket)parsedResult.first).X_GyroRate == 6.78f);
    REQUIRE(((ParsedPacket)parsedResult.first).Y_GyroRate == 98.8f);
    REQUIRE(((ParsedPacket)parsedResult.first).Z_GyroRate == 4.8956f);
    dataBuffer.clear();
}

TEST_CASE("Test multiple full packets and 1 partial packet")
{
    uint8_t position = 0;
    addPacketToBuffer(dataBuffer, 1, 1.0, 1.3, 1.5);
    auto parsedResult = parsePacket(dataBuffer, &position, true);
    REQUIRE(parsedResult.second == true);
    REQUIRE(((ParsedPacket)parsedResult.first).packetCount == 1);
    REQUIRE(((ParsedPacket)parsedResult.first).X_GyroRate == 1.0);
    REQUIRE(((ParsedPacket)parsedResult.first).Y_GyroRate == 1.3f);
    REQUIRE(((ParsedPacket)parsedResult.first).Z_GyroRate == 1.5f);

    addPacketToBuffer(dataBuffer, 3, 7.78, 218.8, 4.8956);
    parsedResult = parsePacket(dataBuffer, &position, true);
    REQUIRE(parsedResult.second == true);
    REQUIRE(((ParsedPacket)parsedResult.first).packetCount == 3);
    REQUIRE(((ParsedPacket)parsedResult.first).X_GyroRate == 7.78f);
    REQUIRE(((ParsedPacket)parsedResult.first).Y_GyroRate == 218.8f);
    REQUIRE(((ParsedPacket)parsedResult.first).Z_GyroRate == 4.8956f);

    dataBuffer.push_back(0x7F);
    dataBuffer.push_back(0xF0);
    dataBuffer.push_back(0x1C);
    dataBuffer.push_back(0xAF);

    uint32_t packetC = htonl((uint32_t)4);
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&packetC);
    for(int i = 0; i < 4; i++) {
        dataBuffer.push_back(bytes[i]);
    }

    uint32_t xGyroBytes;
    float xGyro = 3.4;
    memcpy(&xGyroBytes, &xGyro, sizeof(float));
    xGyroBytes = htonl(xGyroBytes);
    bytes = reinterpret_cast<uint8_t*>(&xGyroBytes);
    for(int i = 0; i < 4; i++) {
        dataBuffer.push_back(bytes[i]);
    }

    uint32_t yGyroBytes;
    float yGyro = 7.89;
    memcpy(&yGyroBytes, &yGyro, sizeof(float));
    yGyroBytes = htonl(yGyroBytes);
    bytes = reinterpret_cast<uint8_t*>(&yGyroBytes);
    for(int i = 0; i < 4; i++) {
        dataBuffer.push_back(bytes[i]);
    }
    std::cout << dataBuffer.size() << std::endl;
    std::cout << PACKET_SIZE << std::endl;
    parsedResult = parsePacket(dataBuffer, &position, true);
    REQUIRE(parsedResult.second == false);
    dataBuffer.clear();
}

TEST_CASE("Test packet starting in the middle")
{
    uint8_t position = 0;
    dataBuffer.push_back(0x95);
    dataBuffer.push_back(0xAB);
    dataBuffer.push_back(0xCD);
    dataBuffer.push_back(0xEE);
    addPacketToBuffer(dataBuffer, 1, 1.0, 1.3, 1.5);
    auto parsedResult = parsePacket(dataBuffer, &position, true);
    REQUIRE(parsedResult.second == true);
    REQUIRE(((ParsedPacket)parsedResult.first).packetCount == 1);
    REQUIRE(((ParsedPacket)parsedResult.first).X_GyroRate == 1.0);
    REQUIRE(((ParsedPacket)parsedResult.first).Y_GyroRate == 1.3f);
    REQUIRE(((ParsedPacket)parsedResult.first).Z_GyroRate == 1.5f);
    dataBuffer.clear();
}

