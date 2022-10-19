/**
 * FILENAME: receiver_main.h
 *
 * DESCRIPTION:
 *
 * DATE: Saturday, Oct 8, 2022
 *
 * AUTHOR:
 *
 */

#ifndef RECEIVER_MAIN_H
#define RECEIVER_MAIN_H

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
#include <errno.h>
#include <fstream>

#define TIMEOUT         1.0
#define DATA_BUF_SIZE   5000

typedef struct {
    char data[DATA_BUF_SIZE];
    int seq_num;
    int size;
} packet;

typedef struct {
    int ack_num;
} ACK;

struct timeval tv;
struct sockaddr_in si_me, si_other, s_addr;
socklen_t s_addrlen = sizeof s_addr;
int sockfd, slen;

#endif
