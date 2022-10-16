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
//#define SLOW_START 0
//#define CONGESTION_AVOIDANCE 1
//#define FAST_RECOVERY 2

enum ctrl_state {SLOW_START, CONGESTION_AVOIDANCE, FAST_RECOVERY};

typedef struct {
    char data[BUFFER_SIZE];
    int seq_num;
    int size;
} packet;

typedef struct {
    int seq_num;
} ACK;

struct sockaddr_in si_other, c_addr;                                                                
socklen_t c_addrlen = sizeof c_addr;
const int MSS = sizeof(char)*BUFFER_SIZE;
int sockfd, slen, n;
int state = SLOW_START;

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
