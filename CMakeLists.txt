cmake_minimum_required(VERSION 3.23)
project(TrueAnomalyAssessment)

set(CMAKE_CXX_STANDARD 14)

find_package(Catch2 3 REQUIRED)

include_directories(IMU_Parser/src)

add_executable(TrueAnomalyAssessment
        IMU_Parser/src/imu_reader.cpp
        IMU_Parser/src/imu_reader.h)

add_executable(test_imu
        IMU_Parser/test/test_imu.cpp
        IMU_Parser/test/fake_data.cpp)
