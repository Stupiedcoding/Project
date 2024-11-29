#include <stdio.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdint.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

// ICMP 헤더 정의
struct icmp_header {
    unsigned char msg_type;
    unsigned char detail_code;
    unsigned short checksum;
    unsigned short icmp_id;
    unsigned short sequence;
};

// IP 헤더 정의
struct ip_header {
    unsigned char  iph_verlen;      // 4 bits for version, 4 bits for header length
    unsigned char  iph_tos;         // Type of Service
    unsigned short iph_len;         // Total length
    unsigned short iph_ident;       // Identification
    unsigned short iph_flagfrag;    // Flags and Fragment offset
    unsigned char  iph_ttl;         // Time to live
    unsigned char  iph_protocol;    // Protocol (ICMP, TCP, UDP, etc.)
    unsigned short iph_chksum;      // Header checksum
    unsigned int   iph_sourceip;    // Source IP address
    unsigned int   iph_destip;      // Destination IP address
};

// ICMP 패킷 정의 (헤더 + 데이터)
struct icmp_packet {
    struct icmp_header ichd;
    char data[64];
};

// 체크섬 계산 함수
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

// ICMP Echo Request 전송 함수
void send_icmp_request(SOCKET raw_socket, const char *dest_ip, int ttl_value, int sequence) {
    struct sockaddr_in dest_addr;
    struct icmp_packet icmp_packet;

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = 0; // ICMP 프로토콜
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);

    setsockopt(raw_socket, IPPROTO_IP, IP_TTL, (const char *)&ttl_value, sizeof(ttl_value));

    memset(&icmp_packet, 0, sizeof(icmp_packet));
    icmp_packet.ichd.msg_type = 8;
    icmp_packet.ichd.detail_code = 0;
    icmp_packet.ichd.icmp_id = htons(1);
    icmp_packet.ichd.sequence = htons(sequence);
    memset(icmp_packet.data, 0, sizeof(icmp_packet.data));
    strcpy(icmp_packet.data, "ICMP Echo Data");
    icmp_packet.ichd.checksum = checksum(&icmp_packet, sizeof(icmp_packet));

    if (sendto(raw_socket, (const char *)&icmp_packet, sizeof(icmp_packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        printf("Sendto failed: %d\n", WSAGetLastError());
        return;
    }
}

void receive_icmp_reply(SOCKET raw_socket, int ttl_value, int *reached_target) {
    struct sockaddr_in receiver_info;

    int from_len = sizeof(receiver_info);
    char recv_buffer[2048];

    if (recvfrom(raw_socket, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&receiver_info, &from_len) == SOCKET_ERROR) {
        int error_code = WSAGetLastError();
        if (error_code == WSAETIMEDOUT) {
            printf("Request time out, no packet received\n");
        } else {
            printf("Recvfrom failed: %d\n", error_code);
        }
        return;
    }

    // IP 헤더를 가져오기
    struct ip_header *receive_ip_address = (struct ip_header *)recv_buffer;

    if (receive_ip_address->iph_protocol == IPPROTO_ICMP) {
        struct icmp_header *reply_header = (struct icmp_header *)(recv_buffer + (receive_ip_address->iph_verlen & 0x0F) * 4);
        if (reply_header->msg_type == 0) {
            printf("IP trace is complete\n");
            *reached_target = 1;
        } else if (reply_header->msg_type == 11) {
            printf("Hop %d: %s\n", ttl_value, inet_ntoa(receiver_info.sin_addr));
        }
    } else {
        printf("Non-ICMP packet received, ignoring...\n");
    }
}

void trace_route(const char *dest_ip) {
    SOCKET raw_socket;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (raw_socket < 0) {
        printf("Socket creation failed\n");
        return;
    }

    // 소켓 타임아웃 설정
    int timeout = 10000;
    setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    for (int ttl = 1; ttl <= 30; ttl++) {
        int tracert_yes_no = 0;
        send_icmp_request(raw_socket, dest_ip, ttl, ttl * 256);
        receive_icmp_reply(raw_socket, ttl, &tracert_yes_no);
        if (tracert_yes_no) {
            closesocket(raw_socket);
            WSACleanup();
            printf("Trace complete.\n");
            return;
        }
    }
    closesocket(raw_socket);
    WSACleanup();
}

int main() {
    const char *dest_ip = "8.8.8.8"; // 목적지 IP
    trace_route(dest_ip);
    return 0;
}