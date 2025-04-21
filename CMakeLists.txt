cmake_minimum_required(VERSION 3.2)
project(TrueAnomalyAssessment)

set(CMAKE_CXX_STANDARD 14)

find_package(Catch2 3 REQUIRED)

include_directories(IMU_Parser/src)

add_executable(imu_parser
        IMU_Parser/src/imu_parser.cpp
        IMU_Parser/src/imu_parser.h
        IMU_Parser/src/parse_functions.cpp
        IMU_Parser/src/parse_functions.h)

add_executable(imu_tester
        IMU_Parser/test/mock_imu.cpp
        IMU_Parser/test/fake_data.cpp)

add_executable(parser_test
        IMU_Parser/test/parser_test.cpp
        IMU_Parser/src/parse_functions.cpp
        IMU_Parser/src/parse_functions.h)
target_link_libraries(parser_test PRIVATE Catch2::Catch2)
