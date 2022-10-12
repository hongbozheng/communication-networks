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

#define DATA            0
#define ACK             2
#define FIN             3
#define FIN_ACK         4
#define DATA_SIZE       2000
#define BUF_SIZE        600000
#define PKT_BUF_SIZE    300

typedef struct {
    int data_size;
    int seq_num;
    int ack_num;
    int msg_type;
    char data[DATA_SIZE];
} packet;

struct sockaddr_in si_me, si_other, sender_addr;
socklen_t addrlen = sizeof sender_addr;
int s, slen, recv_byte;

char buf[sizeof(packet)];

#endif
