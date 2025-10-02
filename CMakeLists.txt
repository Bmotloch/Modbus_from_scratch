cmake_minimum_required(VERSION 3.0)
project(modbus)

# Set compiler flags and options
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Specify the executable
add_executable(modbus main.cpp frame.cpp master.cpp slave.cpp modbus1.cpp)
