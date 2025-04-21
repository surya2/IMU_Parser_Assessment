//
// Created by Surya Selvam on 4/20/2025.
//

#include <iostream>
#include <vector>
#include <random>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
extern "C" {
    uint8_t *createFakePacket(uint32_t count) {
        std::vector<uint8_t> packet;

        /* Append the header bytes */
        packet.push_back(0x7F);
        packet.push_back(0xF0);
        packet.push_back(0x1C);
        packet.push_back(0xAF);

        /* Random number generator for generating random gyro values */
        std::random_device rd;
        std::mt19937 gen(rd());

        uint32_t packetCount = htonl((uint32_t)count); // reorders bytes into big-endian according to IEEE-754 network ordering

        uint8_t *packetCountBytes = reinterpret_cast<uint8_t *>(&packetCount);  // split up into bytes pointed to by pointer
        for (int i = 0; i < 4; i++) {
            packet.push_back(packetCountBytes[i]);
        }

        /* Generate random gyro values */
        std::uniform_real_distribution<float> floats(-100.0f, 100.0f);
        float x_gyro_rate = floats(gen);
        float y_gyro_rate = floats(gen);
        float z_gyro_rate = floats(gen);

        /* Copy the random gyro values (which are floats) byte-by-byte into a 32-bit buffer */
        uint32_t x_gyro_bytes;
        memcpy(&x_gyro_bytes, &x_gyro_rate, sizeof(float));
        x_gyro_bytes = htonl(x_gyro_bytes); // reorder bytes into network byte order/big-endian
        uint8_t* x_gyro_ptr = reinterpret_cast<uint8_t*>(&x_gyro_bytes);
        for (int i = 0; i < 4; i++) {
            packet.push_back(x_gyro_ptr[i]);  // push 32-bits (4 bytes) for gyro value into
        }

        uint32_t y_gyro_bytes;
        memcpy(&y_gyro_bytes, &y_gyro_rate, sizeof(float));
        y_gyro_bytes = htonl(y_gyro_bytes);
        uint8_t* y_gyro_ptr = reinterpret_cast<uint8_t*>(&y_gyro_bytes);
        for (int i = 0; i < 4; i++) {
            packet.push_back(y_gyro_ptr[i]);
        }

        uint32_t z_gyro_bytes;
        memcpy(&z_gyro_bytes, &z_gyro_rate, sizeof(float));
        z_gyro_bytes = htonl(z_gyro_bytes);
        uint8_t* z_gyro_ptr = reinterpret_cast<uint8_t*>(&z_gyro_bytes);
        for (int i = 0; i < 4; i++) {
            packet.push_back(z_gyro_ptr[i]);
        }

        uint8_t* packetPtr = new uint8_t[packet.size()];
        std::copy(packet.begin(), packet.end(), packetPtr);  // This is simply to use the copy constructor to create a pointer pointing to the 'packet' vector as Python requires this when calling an extern function returning some object
        return packetPtr;
    }
    void freePacket(uint8_t* data) {  // To free the vector which takes up heap space
        delete[] data;
    }
}

