#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
//#pragma comment(lib, "ws2_32.lib") if not use CMakeLists you must request linker by pragma comment

struct icmp_header {
    unsigned char msg_type;
    unsigned char detail_code;
    unsigned short checksum;
    unsigned short icmp_id;
    unsigned short sequence;
};

struct ip_header { // using icmp reply
    unsigned char  iph_verlen; //ipv4 (4bit) + IHL(4bit)
    unsigned char  iph_tos;
    unsigned short iph_len; // IP hdr
    unsigned short iph_ident;
    unsigned short iph_flagfrag;
    unsigned char  iph_ttl;
    unsigned char  iph_protocol;// icmp,tcp,udp
    unsigned short iph_chksum; //ip checksum
    unsigned int   iph_sourceip; // router ip
    unsigned int   iph_destip; // node ip(user,router)
};

struct icmp_packet {
    struct icmp_header ichd;
    char data[64];
};

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result = 0;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void send_icmp_request(SOCKET raw_socket, const char *dest_ip, int ttl_value, int sequence) {
    struct sockaddr_in dest_addr;
    struct icmp_packet icmp_packet;

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = 0; // ICMP 프로토콜
    dest_addr.sin_addr.s_addr = inet_addr(dest_ip);

    setsockopt(raw_socket, IPPROTO_IP, IP_TTL, (const char *)&ttl_value, sizeof(ttl_value));

    memset(&icmp_packet, 0, sizeof(icmp_packet)); //padding problem

    icmp_packet.ichd.msg_type = 8;
    icmp_packet.ichd.detail_code = 0;
    icmp_packet.ichd.icmp_id = htons(1);
    icmp_packet.ichd.sequence = htons(sequence);
    memset(icmp_packet.data, 0, sizeof(icmp_packet.data));
    strcpy(icmp_packet.data, "ICMP Echo Data");
    icmp_packet.ichd.checksum = checksum(&icmp_packet, sizeof(icmp_packet));

    if (sendto(raw_socket, (const char *)&icmp_packet, sizeof(icmp_packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) < 0) {
        printf("sendfailed: %d\n", WSAGetLastError());
        exit(1);
        return;
    }
}
int receive_icmp_reply(SOCKET raw_socket, int ttl_value, int *reached_target) {
    struct sockaddr_in receiver_info;
    int from_len = sizeof(receiver_info);
    char recv_buffer[2048];

    if (recvfrom(raw_socket, recv_buffer, sizeof(recv_buffer), 0, (struct sockaddr *)&receiver_info, &from_len) == SOCKET_ERROR) { //SOCKET_ERROR == -1
        int error_code = WSAGetLastError();
        if (error_code == WSAETIMEDOUT) { // if(error_code == 10060)
            printf("Request time out, no packet received\n"); //
        } else {
            if(error_code == WSAEINVAL) // if(error_code == 10022)
            printf("Recvfrom failed: %d\n", error_code);
        }
        return 1;
    }
    struct ip_header *receive_ip_address = (struct ip_header *)recv_buffer;

    if (receive_ip_address->iph_protocol == IPPROTO_ICMP) {
        struct icmp_header *ttl = (struct icmp_header *)recv_buffer;
        struct icmp_header *reply_header = (struct icmp_header *)(recv_buffer + (receive_ip_address->iph_verlen & 0x0F) * 4); //IHL block(4byte) express digit
        if (reply_header->msg_type == 0) {
            printf("IP %s trace is complete ttl : %d\t",inet_ntoa(receiver_info.sin_addr),receive_ip_address->iph_ttl);
            *reached_target = 1;
        } else if (reply_header->msg_type == 11) {
            printf("Hop %d: %s\n", ttl_value, inet_ntoa(receiver_info.sin_addr));
            return 0;
        }
    } else {
        printf("Non-ICMP packet received, ignoring...\n");
        return 1;
    }
}

void DNS_to_ip(char *Dest_IP_OR_DNS) {
    struct sockaddr_in dest_addr;
    struct hostent *host_entry = gethostbyname(Dest_IP_OR_DNS);
    if(inet_addr(Dest_IP_OR_DNS) != INADDR_NONE) {
        return;
    }
    if (host_entry == NULL) {
        printf("Host not found\n");
        return;
    }
    else {
        memcpy(&dest_addr.sin_addr, host_entry->h_addr_list[0], sizeof(dest_addr.sin_addr));
        strcpy(Dest_IP_OR_DNS, inet_ntoa(dest_addr.sin_addr));
    }

}

void trace_route(char *dest_ip,int input_ttl) {
    SOCKET raw_socket;
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    LARGE_INTEGER  start, end,frequency; //integer 64bit
    QueryPerformanceFrequency(&frequency); // 10MHz
    //printf("Frequency: %lld counts per second\n", frequency.QuadPart);
    if (raw_socket < 0) {
        printf("Socket creation failed\n");
        return;
    }

    int timeout = 3000;
    double stop_watch=0;
    setsockopt(raw_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    DNS_to_ip(dest_ip);
    //printf("Destination IP: %s\n", dest_ip); checking code

    if(input_ttl == 0) {
        for (int ttl = 1; ttl <= 30; ttl++) {
            int tracert_yes_no = 0;
            QueryPerformanceCounter(&start); //DWORD is UNSIGNED LONG, timing hardware example system clock reset timeing: computer re-booting
            send_icmp_request(raw_socket, dest_ip, ttl, ttl * 256);
            if(receive_icmp_reply(raw_socket, ttl, &tracert_yes_no)==0) {
                QueryPerformanceCounter(&end);
                stop_watch = ((double)(end.QuadPart - start.QuadPart) / (double)frequency.QuadPart)*1000;
                printf("time : %.4f ms\n", stop_watch);
            }
            //printf("Frequency: %lld counts per second\n", start.QuadPart); checking code
            //printf("Frequency: %lld counts per second\n", end.QuadPart); checking code
            if (tracert_yes_no) {
                closesocket(raw_socket);
                WSACleanup();
                printf("Trace complete.\n");
                return;
            }
        }
    }
    else {
        double avg_time=0;
        double max_time=0;
        double min_time=0;
        for(int i = 1; i <= 10; i++) {
            QueryPerformanceCounter(&start);
            send_icmp_request(raw_socket, dest_ip, input_ttl, input_ttl * 256);
            receive_icmp_reply(raw_socket, input_ttl, 0);
            QueryPerformanceCounter(&end);
            stop_watch += ((double)(end.QuadPart - start.QuadPart) / (double)frequency.QuadPart)*1000;
            if(min_time==0 && max_time==0) {
                min_time = stop_watch;
                max_time = stop_watch;
            }
            else if(min_time > stop_watch) {
                min_time = stop_watch;
            }
            else if(max_time < stop_watch) {
                max_time = stop_watch;
            }
        }
        printf("time : %.4f ms, max : %.4f ms , min : %.4f ms",stop_watch/10,max_time,min_time);
    }
    closesocket(raw_socket);
    WSACleanup();
}

int main(){
    char dest_ip[16] ; // destination IP or DNS
    int input_ttl=0;
    scanf("%s %d",dest_ip,&input_ttl);
    trace_route(dest_ip,input_ttl);
    return 0;
}