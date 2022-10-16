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
#include <fstream>                                                                                  
#include <iostream>
#include <errno.h>

#define TIMEOUT_SECONDS 1                                                                           
#define MAXDATASIZE 1100 // max number of bytes we can get at once
#define CONTENT_BUFFER_SIZE 1200

typedef struct {
    char content[CONTENT_BUFFER_SIZE];
    int seq_num;
    int bytes_read;
} packet;

typedef struct {
    int ack_num;
} ACK;

using namespace std;
struct timeval tv;
struct sockaddr_in si_me, si_other, s_addr;
socklen_t s_addrlen = sizeof s_addr;
int sockfd, slen, n;

/*
#define DATA            0
#define ACK             2
#define FIN             3
#define FIN_ACK         4
#define DATA_SIZE       2000
#define PKT_BUF_SIZE    300
#define BUF_SIZE        600000

typedef struct {
    int data_size;
    int seq_num;
    int ack_num;
    int msg_type;
    char data[DATA_SIZE];
} packet;

struct sockaddr_in si_me, si_other, sender_addr;
socklen_t addrlen = sizeof sender_addr;
int s, slen;

char buf[sizeof(packet)];
*/
#endif
