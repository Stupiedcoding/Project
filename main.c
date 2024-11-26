#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <time.h>
#include <pcap.h> // 소켓 오픈
#include <string.h>

#pragma comment(lib, "ws2_32.lib")

#define ETHERNET_HEADER_SIZE 14
#define IP_MAXPACKET 65535

struct iphdr {
    uint8_t ihl : 4;
    uint8_t version : 4;
    uint8_t tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t frag_off;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t check;
    uint32_t saddr;
    uint32_t daddr;
};

// ICMP 헤더 구조체 정의
struct icmphdr {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t id;
    uint16_t sequence;
};

// 체크섬 계산 함수
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++; //

    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void build_icmp_packet(uint8_t *packet, const char *src_ip, const char *dest_ip, int ttl) {
    struct iphdr *ip_header = (struct iphdr *)packet;
    struct icmphdr *icmp_header = (struct icmphdr *)(packet + sizeof(struct iphdr));

    ip_header->ihl = 5;
    ip_header->version = 4;
    ip_header->tos = 0;
    ip_header->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr));
    ip_header->id = htons(54321);
    ip_header->frag_off = 0;
    ip_header->ttl = ttl;
    ip_header->protocol = IPPROTO_ICMP;
    ip_header->saddr = inet_addr(src_ip);
    ip_header->daddr = inet_addr(dest_ip);
    ip_header->check = 0;
    ip_header->check = checksum((unsigned short *)ip_header, sizeof(struct iphdr));

    icmp_header->type = 8; // Echo Request
    icmp_header->code = 0;
    icmp_header->checksum = 0;
    icmp_header->id = htons(12345);
    icmp_header->sequence = htons(ttl);
    icmp_header->checksum = checksum((unsigned short *)icmp_header, sizeof(struct icmphdr));
}

void send_packet(pcap_t *handle, char *packet, int packet_size) {
    int check = pcap_sendpacket(handle, (const u_char *)packet, packet_size);
    if (check != 0) {
        printf("Error sending packet: %s\n", pcap_geterr(handle));
        printf(check);
    }
}
void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
    struct iphdr *ip_header = (struct iphdr *)(packet + ETHERNET_HEADER_SIZE);
    struct icmphdr *icmp_header = (struct icmphdr *)(packet + ETHERNET_HEADER_SIZE + ip_header->ihl * 4);

    if (icmp_header->type == 0) { // Echo Reply
        printf("Echo Reply from %s\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr));
    } else if (icmp_header->type == 11) { // TTL Exceeded
        printf("TTL Exceeded from %s\n", inet_ntoa(*(struct in_addr *)&ip_header->saddr));
    }
}

// Tracert 구현 함수
void tracert(const char *src_ip, const char *dest_ip) {
    pcap_if_t *all_devices, *device;
    char errbuf[PCAP_ERRBUF_SIZE];

    // 디바이스 목록 가져오기
    if (pcap_findalldevs(&all_devices, errbuf) == -1) {
        fprintf(stderr, "Error finding devices: %s\n", errbuf);
        return;
    }

    device = all_devices;
    if (device == NULL) {
        fprintf(stderr, "No devices found.\n");
        return;
    }

    // 첫 번째 디바이스 선택
    pcap_t *handle = pcap_open_live(device->name, 65536, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Error opening device: %s\n", errbuf);
        pcap_freealldevs(all_devices);
        return;
    }
    printf("please work3");
    uint8_t packet[IP_MAXPACKET];
    for (int ttl = 1; ttl <= 30; ttl++) {
        memset(packet, 0, sizeof(packet));

        // ICMP 패킷 작성
        build_icmp_packet(packet, src_ip, dest_ip, ttl);

        // 패킷 전송
        if (pcap_sendpacket(handle, packet, sizeof(struct iphdr) + sizeof(struct icmphdr)) != 0) {
            fprintf(stderr, "Error sending packet: %s\n", pcap_geterr(handle));
            break;
        }

        // 패킷 수신
        struct pcap_pkthdr *header;
        const u_char *recv_packet;
        int result = pcap_next_ex(handle, &header, &recv_packet);
        if (result == 1) {
            packet_handler(NULL, header, recv_packet);
        } else if (result == 0) {
            printf("Timeout waiting for packet.\n");
        } else {
            fprintf(stderr, "Error capturing packet: %s\n", pcap_geterr(handle));
        }

    }

    // 종료
    pcap_close(handle);
    pcap_freealldevs(all_devices);
}
void get_local_ip(char *ip) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    char host[256];
    struct hostent *he;

    /*struct hostent {
    char *h_name; // 공식 호스트 이름
    char **h_aliases; // 별칭
    int h_addrtype; // AF_INET, AF_INET6
    int h_length; // 주소길이
    char **h_addr_list; // 호스트의 IP 주소 가리키는 변수에는 배열로 되어있음
    */

    struct in_addr addr;
    /*
    struct in_addr {
        uint32_t s_addr;  // ipv4 ip
    };
    */
    gethostname(host, sizeof(host)); //호스트 이름을 저장할 버퍼변수와 그 버퍼의 크기
    he = gethostbyname(host); // 호스트이름을 기반으로 정보를 가져옴 이때 hostent 구조체 반환

    if (he == NULL) {
        printf("Unable to get host information\n");
        return;
    }

    addr.s_addr = *he->h_addr_list[0]; //구조체 중에 실질적 ip값을 가진 변수의 포인터들을 배열을 가진 변수에서 ip에 대한 포인터를 역참조  addr.s_addr에 저장
    strcpy(ip, inet_ntoa(addr)); // ip를 가진 addr를 local_ip배열에 넣는것
    WSACleanup();
}

int main() {
    char local_ip[16];
    get_local_ip(local_ip);
    const char *src_ip = local_ip;
    const char *dest_ip = "8.8.8.8";
    tracert(src_ip, dest_ip);
    return 0;
}