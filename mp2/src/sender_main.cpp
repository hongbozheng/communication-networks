/** 
 * FILENAME: sender_main.c
 * 
 * DESCRIPTION:
 *
 * DATE: Saturday, Oct 8th, 2022
 *
 * AUTHOR:
 *
 */

#include "sender_main.h"

//struct sockaddr_in si_other;
//int s, slen;

void diep(char *s) {
    perror(s);
    exit(1);
}

int get_socket(char * hostname, unsigned short int hostUDPport) {
    int rv, sockfd;
    char port[10];
    sprintf(port, "%d", hostUDPport);
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    memset(&recvinfo,0,sizeof recvinfo);
    if ((rv = getaddrinfo(hostname, port, &hints, &recvinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = recvinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            printf("[ERROR]: Cannot open the socket\n");
            continue;
        }
        break;
    }
    if (p == NULL)  {
        printf("[ERROR]: Server failed to bind\n");
        exit(1);
    }

    return sockfd;
}

void set_socket_timeout(int sockfd){
    struct timeval TIMEOUT;
    TIMEOUT.tv_sec = 0;
    TIMEOUT.tv_usec = 2*RTT;
    if (setsockopt(sockfd, SOL_SOCKET,SO_RCVTIMEO,&TIMEOUT,sizeof(TIMEOUT)) == -1) {
        printf("[ERROR]: Failed to set socket timeout\n");
        return;
    }
}

void create_pkt_queue(int pkt_number, FILE *fp) {
    if (pkt_number == 0) return;
    int pkt_data_byte;
    char buf[MSS];
    
    for (int i = 0; bytesToRead != 0 && i < pkt_number; ++i) {
        packet pkt;
        if (bytesToRead >= MSS) {
            pkt_data_byte = MSS;
        } else {
            pkt_data_byte = bytesToRead;
        }

        int byte_read = fread(buf, sizeof(char), pkt_data_byte, fp);
        if (byte_read > 0) {
            pkt.data_size = byte_read;
            pkt.msg_type = DATA;
            pkt.seq_num = seq_number;
            memcpy(pkt.data, &buf, sizeof(char)*pkt_data_byte);
            pkt_queue.push(pkt);
            seq_number = (seq_number + 1) % MAX_SEQ_NUMBER;
        } else {
            printf("[INFO]: Reach EOF, fread 0 byte\n");
        }
        bytesToRead -= byte_read;
    }
}

int send_pkt(int sockfd) {
    if(pkt_queue.empty()) {
        printf("[INFO]: No packet(s) need to be sent\n");
        return 0;
    }
    
    int send_pkt_num  = (cwnd - wait_ack.size()) <= pkt_queue.size() ? cwnd - wait_ack.size() : pkt_queue.size();
    int byte_send;
    
    if(cwnd - wait_ack.size() < 1) {
        memcpy(pkt_buf, &wait_ack.front(), sizeof(packet));
        if((byte_send = sendto(sockfd, pkt_buf, sizeof(packet), 0, p->ai_addr, p->ai_addrlen))== -1){
            printf("Fail to send %d pkt\n", wait_ack.front().seq_num);
            exit(2);
        }
        printf("[INFO]: Send packet %d, cwnd = %f\n", wait_ack.front().seq_num, cwnd);
        return 0;
    }

    for(int i = 0; i < send_pkt_num; ++i) {
        memcpy(pkt_buf, &pkt_queue.front(), sizeof(packet));
        if((byte_send = sendto(sockfd, pkt_buf, sizeof(packet), 0, p->ai_addr, p->ai_addrlen)) == -1){
            printf("Fail to send packet %d\n", pkt_queue.front().seq_num);
            exit(2);
        }
        printf("[INFO]: Sent packet %d successfully, cwnd = %f\n", pkt_queue.front().seq_num, cwnd);
        wait_ack.push(pkt_queue.front());
        pkt_queue.pop();
    }
    //create_pkt_queue(pkts_to_send, fp);
    return send_pkt_num;
}

void state_ctrl(bool newACK, bool timeout) {
    switch (ctrl_state) {
        case SLOW_START:
            if (timeout) {
                ssthread = cwnd/2.0;
                cwnd = 1;
                dup_ack_cnt = 0;
                break;
            }
            if (newACK) {
                dup_ack_cnt = 0;
                cwnd = (cwnd+1 >= BUFFER_SIZE) ? BUFFER_SIZE-1 : cwnd+1;
            } else {
                ++dup_ack_cnt;
            }
            if (cwnd >= ssthread) {
                printf("[INFO]: SLOW_START ---> CONGESTION_AVOIDANCE, cwnd = %f\n", cwnd);
                ctrl_state = CONGESTION_AVOIDANCE;
            }
            break;
        case CONGESTION_AVOIDANCE:
            if (timeout) {
                ssthread = cwnd/2.0;
                cwnd = 1;
                dup_ack_cnt= 0;
                printf("[INFO]: CONGESTION_AVOIDANCE ---> SLOW_START, cwnd = %f\n", cwnd);
                ctrl_state = SLOW_START;
                break;
            }
            if (newACK) {
                cwnd = (cwnd+ 1.0/cwnd >= BUFFER_SIZE) ? BUFFER_SIZE-1 : cwnd+ 1.0/cwnd;
                dup_ack_cnt = 0;
            } else {
                ++dup_ack_cnt;
            }
            break;
        case FAST_RECOVERY:
            if (timeout) {
                ssthread = cwnd/2.0;
                cwnd = 1;
                dup_ack_cnt = 0;
                printf("[INFO]: FAST_RECOVERY ---> SLOW_START, cwnd = %f\n", cwnd);
                ctrl_state = SLOW_START;
                break;
            }
            if (newACK) {
                cwnd = ssthread;
                dup_ack_cnt = 0;
                printf("[INFO]: FAST_RECOVERY ---> CONGESTION_AVOIDANCE, cwnd = %f\n", cwnd);
                ctrl_state = CONGESTION_AVOIDANCE;
            } else {
                cwnd = (cwnd+1 >= BUFFER_SIZE) ? BUFFER_SIZE-1 : cwnd+1;
            }
            break;
        default:
            break;
    }
}

void fin_ack(int sockfd) {
    packet pkt;
    int byte_num;

    while(1) {
        pkt.data_size = 0;
        pkt.seq_num = seq_number;
        pkt.msg_type = FIN;
        memcpy(pkt_buf, &pkt, sizeof(packet));
        if((byte_num = sendto(sockfd, pkt_buf, sizeof(packet), 0, p->ai_addr, p->ai_addrlen))== -1){
            printf("[ERROR]: Failed to send FIN to receiver\n");
            exit(2);
        }
        if ((byte_num = recvfrom(sockfd, pkt_buf, sizeof(packet), 0, (struct sockaddr *) &their_addr, &addr_len)) == -1) {
            printf("[ERROR]: Failed to receive ACK from receiver\n");
            exit(2);
        }
        memcpy(&pkt, pkt_buf, sizeof(packet));
        if (pkt.msg_type == FIN_ACK) {
            printf("[INFO]: Receive the FIN_ACK\n");
            break;
        }
    }
}

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {
    /*
    //Open the file
    FILE *fp;
    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not open file to send.");
        exit(1);
    }

	// Determine how many bytes to transferS

    slen = sizeof (si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }*/


	/* Send data and receive acknowledgements on s*/

	int sockfd = get_socket(hostname, hostUDPport);
    set_socket_timeout(sockfd);
    
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Could not open file to send.");
        exit(1);
    }
    bytesToRead = bytesToTransfer;

    unsigned long long int pkt_total = (unsigned long long int) ceil((float)bytesToRead / MSS);
    printf("[INFO]: %llu packet(s) need to be sent\n", pkt_total);

    create_pkt_queue(BUFFER_SIZE, fp);
    
    int send_pkt_num;
    send_pkt_num = send_pkt(sockfd);
    create_pkt_queue(send_pkt_num, fp);

    int byte_num;

    while (!pkt_queue.empty() || !wait_ack.empty()) {
        if((byte_num = recvfrom(sockfd, pkt_buf, sizeof(packet), 0, NULL, NULL)) == -1) {
            if (errno != EAGAIN || errno != EWOULDBLOCK) {
                printf("[INFO]: Fail to receive main ACK\n");
                exit(2);
            }
            printf("[INFO]: Timeout, resend packet %d\n", wait_ack.front().seq_num);
            memcpy(pkt_buf, &wait_ack.front(), sizeof(packet));
            if((byte_num = sendto(sockfd, pkt_buf, sizeof(packet), 0, p->ai_addr, p->ai_addrlen))== -1) {
                printf("Fail to send %d pkt\n", wait_ack.front().seq_num);
                exit(2);
            }
            state_ctrl(false, true);
        } else {
            packet pkt;
            memcpy(&pkt, pkt_buf, sizeof(packet));
            if (pkt.msg_type == ACK) {
                printf("[INFO]: Receive ACK from receiver\n");
                printf("        Receiver receives packet %d successfully\n", pkt.ack_num);
                if (pkt.ack_num == wait_ack.front().seq_num) {
                    state_ctrl(false, false);
                    if (dup_ack_cnt == 3) {
                        ssthread = cwnd/2.0;
                        cwnd = ssthread + 3;
                        dup_ack_cnt = 0;
                        ctrl_state = FAST_RECOVERY;
                        printf("[INFO]: Receive 3 duplicate ACK ---> FAST_RECOVERY, cwnd = %f\n", cwnd);
                        // resend duplicated pkt
                        memcpy(pkt_buf, &wait_ack.front(), sizeof(packet));
                        if((byte_num = sendto(sockfd, pkt_buf, sizeof(packet), 0, p->ai_addr, p->ai_addrlen)) == -1) {
                            printf("Fail to resend packet %d\n", wait_ack.front().seq_num);
                            exit(2);
                        }
                        printf("[INFO]: Receive 3 duplicate ACK, resend packet %d\n", wait_ack.front().seq_num);
                    }
                } else if (pkt.ack_num > wait_ack.front().seq_num) {
                    while (!wait_ack.empty() && wait_ack.front().seq_num < pkt.ack_num) {
                        state_ctrl(true, false);
                        wait_ack.pop();
                    }
                    send_pkt_num = send_pkt(sockfd);
                    create_pkt_queue(send_pkt_num, fp);
                }
            }
        }
        printf("----------------------------------------------------------------------\n");
    }
    fclose(fp);

    fin_ack(sockfd);

    printf("[INFO]: Closing the socket\n");
    close(s);
    return;
}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;
    unsigned long long int numBytes;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);

    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);

    return (EXIT_SUCCESS);
}
