cmake_minimum_required(VERSION 3.10)
project(RaspberryWebServer)

set(CMAKE_CXX_STANDARD 11)

add_definitions(-DMG_ENABLE_POSIX=1)

add_executable(server main.cpp logic.cpp mongoose.c)

target_link_libraries(server pthread)