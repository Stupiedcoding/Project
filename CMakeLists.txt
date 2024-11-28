set(CMAKE_C_STANDARD_REQUIRED YES)

cmake_minimum_required(VERSION 3.29)
project(Project)  # 프로젝트 이름 설정

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 6.1)  # Windows 7 이상
set(CMAKE_GENERATOR_PLATFORM x64)

# Npcap 헤더 파일 경로 추가
include_directories("C:/Program Files/Npcap/Include")

# Npcap 라이브러리 경로 추가
link_directories("C:/Program Files/Npcap/Lib")

# pcap 라이브러리 파일 찾기
find_library(PCAP_LIBRARY NAMES wpcap pcap HINTS "C:/Program Files/Npcap/Lib")

# 실행 파일 생성
add_executable(your_program_name main.c)

# Npcap 라이브러리 연결
target_link_libraries(your_program_name ${PCAP_LIBRARY} ws2_32)