cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

project(CAT24C32)

add_library(CAT24C32 STATIC CAT24C32.cpp)

target_link_libraries(CAT24C32 pico_stdlib hardware_i2c)
