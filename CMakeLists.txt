
cmake_minimum_required(VERSION 3.29)

project(Project)

set(CMAKE_GENERATOR_PLATFORM x64)

add_executable(your_program_name icmp.c)

target_link_libraries(your_program_name PRIVATE ws2_32)