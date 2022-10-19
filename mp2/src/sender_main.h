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
#include <deque>
#include <unordered_map>

#define BUFFER_SIZE     5000
#define INIT_SSTHRESH   750000
#define TIMEOUT         1.0
const int MSS = sizeof(char)*BUFFER_SIZE;
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
int sockfd, slen, n;
int cwnd_byte = MSS;
int ssthreash = INIT_SSTHRESH;
int state = SLOW_START;

#endif
