cmake_minimum_required(VERSION 3.21)
project(1C_Test_Task)

set(CMAKE_CXX_STANDARD 20)

find_package(OpenSSL REQUIRED)

add_executable(program
        main.cpp)
target_link_libraries(program OpenSSL::SSL OpenSSL::Crypto)
