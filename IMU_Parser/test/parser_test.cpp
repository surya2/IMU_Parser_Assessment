//
// Created by Surya Selvam on 4/20/2025.
//

#include <catch2/catch_all.hpp>
#include <mutex>
#include <thread>
#include <vector>
#include "../src/imu_reader.h"

std::vector<char> dataBuffer;
std::mutex dataBufferMtx;

TEST_CASE("Test single packet")
{
    dataBuffer.push_back(0x7F);
    dataBuffer.push_back(0xF0);
    dataBuffer.push_back(0x1C);
    dataBuffer.push_back(0xAF);
    dataBuffer.push_back(0x00000001);
    dataBuffer.push_back(0x00000001);
}