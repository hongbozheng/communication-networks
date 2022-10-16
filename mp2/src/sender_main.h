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
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>          // EAGAIN/EWOULDBLOCK
#include <chrono>           // std::time
#include <unordered_map>
#include <deque>
#include <algorithm>
#include <iostream>

#define BUFFER_SIZE 1200
#define INIT_SSTHRESH 64000
#define TIMEOUT_SECONDS 1
#define SLOW_START 0
#define CONGESTION_AVOIDANCE 1
#define FAST_RECOVERY 2
#define SEQUENCE_NUM_INIT 0

char read_buffer[BUFFER_SIZE];
char ack_buffer[3];
struct sockaddr_in si_other, c_addr;                                                                

int sockfd, slen, n, transferredBytes, dup_ACK, ssthreash;
socklen_t c_addrlen;

struct Packet {
    char content[BUFFER_SIZE];
    int seq_num;
    int bytes_read;
};

struct ACK_MSG {
    int ack_num;
};

int MSS = sizeof(char)*BUFFER_SIZE;

/*
//packet structure used for transfering
typedef struct{
    int 	data_size;
    int 	seq_num;
    int     ack_num;
    int     msg_type; //DATA 0 SYN 1 ACK 2 FIN 3 FINACK 4
    char    data[MSS];
}packet;
*/

#endif
