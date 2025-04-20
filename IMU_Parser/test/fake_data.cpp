//
// Created by Surya Selvam on 4/20/2025.
//

#include "fake_data.h"

std::vector<uint8_t> createFakePacket() {
    std::vector<uint8_t> packet;

    packet.push_back(0x7F);
    packet.push_back(0xF0);
    packet.push_back(0x1C);
    packet.push_back(0xAF);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> counts(0, 100);

    uint32_t packetCount = htonl((uint32_t)counts(gen)); // reorders bytes into big-endian according to IEEE-754 network ordering

    uint8_t *packetCountBytes = reinterpret_cast<uint8_t *>(&packetCount);  // split up into bytes pointed to by pointer
    for (int i = 0; i < 4; i++) {
        packet.push_back(packetCountBytes[i]);
    }

    std::uniform_real_distribution<float> floats(-100.0f, 100.0f);

    float x_gyro_rate = floats(gen);
    float y_gyro_rate = floats(gen);
    float z_gyro_rate = floats(gen);


    uint32_t x_gyro_bytes;
    memcpy(&x_gyro_bytes, &x_gyro_rate, sizeof(float));
    x_gyro_bytes = htonl(x_gyro_bytes); // reorder bytes into network byte order/big-endian
    uint8_t* x_gyro_ptr = reinterpret_cast<uint8_t*>(&x_gyro_bytes);
    for (int i = 0; i < 4; i++) {
        packet.push_back(x_gyro_ptr[i]);
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

    return packet;
}
