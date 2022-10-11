/**
 * FILENAME: sender_main.h
 *
 * DESCRIPTION:
 *
 * DATE: Saturday, Oct 8th, 2022
 *
 * AUTHOR:
 *
 */

#ifndef SENDER_MAIN_H
#define SENDER_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <signal.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>      // struct addrinfo
#include <stdbool.h>    // bool var
#include <math.h>       // ceil fn
#include <errno.h>      // EAGAIN/EWOULDBLOCK
#include <queue>        // queue<packet>

#define MSS 2000                // maximum segment size
#define BUFFER_SIZE 300
#define DATA 0
#define ACK 2
#define FIN 3
#define FIN_ACK 4
#define MAX_SEQ_NUMBER 100000
#define RTT 20*1000

// packet for data transfer
typedef struct{
    int 	data_size;
    int 	seq_num;
    int     ack_num;
    int     msg_type; //DATA 0 SYN 1 ACK 2 FIN 3 FINACK 4
    char    data[MSS];
} packet;

unsigned long long int bytesToRead;
//int file_point = 0;

// socket
// I don't think we need the receiver's address info, use NULL instead
//struct sockaddr_storage recv_addr; // connector's address information
//socklen_t addr_len = sizeof recv_addr;
struct addrinfo hints, *recvinfo, *p;
int numbytes;

// Congestion Control
double cwnd = 1.0;
int ssthread = 64, dupAckCount = 0;
enum socket_state {SLOW_START, CONGESTION_AVOIDANCE, FAST_RECOVERY, FIN_WAIT};
int congetion_ctrl_state = SLOW_START;

// slide window
unsigned long long int seq_number;
char pkt_buf[sizeof(packet)];
std::queue<packet> buffer;
std::queue<packet> wait_ack;


int getSocket(char *hostname, unsigned short int hostUDPport);
void openFile(char* filename, unsigned long long int bytesToTransfer);
void congestionControl(bool newACK, bool timeout);
int fillBuffer(int pkt_number);
void sendPkts(int socket);
void setSockTimeout(int socket);

struct sockaddr_in si_other;
int s, slen;

#endif
