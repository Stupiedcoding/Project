set(CMAKE_C_STANDARD_REQUIRED YES)

cmake_minimum_required(VERSION 3.29)

project(Project)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 6.1)
set(CMAKE_GENERATOR_PLATFORM x64)

add_executable(your_program_name main.c)

target_link_libraries(your_program_name ws2_32)