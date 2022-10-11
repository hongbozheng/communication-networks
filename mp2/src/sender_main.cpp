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

// from MP1
int get_socket(char *hostname, unsigned short int hostUDPport) {
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
            //perror("server: error opening socket");
            printf("[ERROR]: Cannot opening socket\n");
            continue;
        }
        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "[SERVER]: Failed to bind\n");
        exit(1);
    }

    return sockfd;
}

int fillBuffer(int pkt_number, FILE *fp) {
    if (pkt_number == 0) return 0;
    int byte_of_pkt;
    char buf[MSS];
    int count = 0;

    for (int i = 0; bytesToRead!= 0 && i < pkt_number; ++i) {
        packet pkt;
        if (bytesToRead >= MSS) {
            byte_of_pkt = MSS;
        } else {
            byte_of_pkt = bytesToRead;
        }
        int byte_read = fread(buf, sizeof(char), byte_of_pkt, fp);
        if (byte_read == byte_of_pkt) {
            pkt.data_size = byte_read;
            pkt.msg_type = DATA;
            pkt.seq_num = seq_number;
            memcpy(pkt.data, &buf, sizeof(char)*byte_of_pkt);
            buffer.push(pkt);
            seq_number = (seq_number + 1) % MAX_SEQ_NUMBER;
        } else {
            printf("[ERROR]: Read file error\n");
        }
        bytesToRead -= byte_read;
        count = i;
    }
    return count;
}

void set_socket_timeout(int socket){
    // Reference: https://manpages.ubuntu.com/manpages/impish/man3/timeval.3bsd.html
    struct timeval TIMEOUT;
    TIMEOUT.tv_sec = 0;
    TIMEOUT.tv_usec = 2*RTT;
    // Reference: https://pubs.opengroup.org/onlinepubs/000095399/functions/setsockopt.html
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &TIMEOUT, sizeof(TIMEOUT)) == -1) {
        fprintf(stderr, "Failed to set socket timeout\n");
        return;
    }
}

void sendPkts(int socket, FILE *fp) {

    int pkts_to_send =(cwnd - wait_ack.size()) <= buffer.size() ? cwnd - wait_ack.size() : buffer.size();
    if (cwnd - wait_ack.size() < 1) {
        memcpy(pkt_buf, &wait_ack.front(), sizeof(packet));
        if((numbytes = sendto(socket, pkt_buf, sizeof(packet), 0, p->ai_addr, p->ai_addrlen))== -1){
            printf("[ERROR]: Fail to send packet %d\n", wait_ack.front().seq_num);
            exit(2);
        }
        printf("[INFO]: Sent packet %d, cwnd = %f\n", wait_ack.front().seq_num, cwnd);
        return;
    }
    if (buffer.empty()) {
        printf("[INFO]: No packet to send\n");
        return;
    }

    for (int i = 0; i < pkts_to_send; ++i) {
        memcpy(pkt_buf, &buffer.front(), sizeof(packet));
        if((numbytes = sendto(socket, pkt_buf, sizeof(packet), 0, p->ai_addr, p->ai_addrlen))== -1){
            perror("Error: data sending");
            printf("[ERROR]: Fail to send %d pkt\n", buffer.front().seq_num);
            exit(2);
        }
        printf("[INFO]: Send packet %d, cwnd = %f\n", buffer.front().seq_num, cwnd);
        wait_ack.push(buffer.front());
        buffer.pop();
    }
    fillBuffer(pkts_to_send, fp);
}


void congestionControl(bool newACK, bool timeout) {
    switch (congetion_ctrl_state) {
        case SLOW_START:
            if (timeout) {
                ssthread = cwnd/2.0;
                cwnd = 1;
                dupAckCount = 0;
                return;
            }
            if (newACK) {
                dupAckCount = 0;
                cwnd = (cwnd+1 >= BUFFER_SIZE) ? BUFFER_SIZE-1 : cwnd+1;
            } else {
                dupAckCount++;
            }
            if (cwnd >= ssthread) {
                printf("[INFO]: SLOW_START to CONGESTION_AVOIDANCE, cwnd = %f\n", cwnd);
                //cout << "SLOW_START to CONGESTION_AVOIDANCE, cwnd = " << cwnd <<endl;
                congetion_ctrl_state = CONGESTION_AVOIDANCE;
            }
            break;
        case CONGESTION_AVOIDANCE:
            if (timeout) {
                ssthread = cwnd/2.0;
                cwnd = 1;
                dupAckCount = 0;
                printf("[INFO]: CONGESTION_AVOIDANCE to SLOW_START, cwnd = %f\n", cwnd);
                //cout << "CONGESTION_AVOIDANCE to SLOW_START, cwnd=" << cwnd <<endl;
                congetion_ctrl_state = SLOW_START;
                return;
            }
            if (newACK) {
                cwnd = (cwnd+ 1.0/cwnd >= BUFFER_SIZE) ? BUFFER_SIZE-1 : cwnd+ 1.0/cwnd;
                dupAckCount = 0;
            } else {
                dupAckCount++;
            }
            break;
        case FAST_RECOVERY:
            if (timeout) {
                ssthread = cwnd/2.0;
                cwnd = 1;
                dupAckCount = 0;
                //cout << "FAST_RECOVERY is SLOW_START, cwnd= " << cwnd <<endl;
                congetion_ctrl_state = SLOW_START;
                return;
            }
            if (newACK) {
                cwnd = ssthread;
                dupAckCount = 0;
                printf("[INFO]: FAST_RECOVERY is CONGESTION_AVOIDANCE, cwnd = %f\n", cwnd);
                //cout << "FAST_RECOVERY is CONGESTION_AVOIDANCE, cwnd = " << cwnd << endl;
                congetion_ctrl_state = CONGESTION_AVOIDANCE;
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
    int num_byte;
    
    while(1) {
        pkt.data_size = 0;
        pkt.seq_num = seq_number;
        pkt.msg_type = FIN;
        memcpy(pkt_buf, &pkt, sizeof(packet));

        if((num_byte = sendto(sockfd, pkt_buf, sizeof(packet), 0, p->ai_addr, p->ai_addrlen)) == -1){
            printf("[ERROR]: Failed to send FIN to receiver\n");
            exit(2);
        }
        // I don't think we need the receiver's address info, use NULL instead
        // if ((num_byte = recvfrom(sockfd, pkt_buf, sizeof(packet), 0, (struct sockaddr *) &recv_addr, &addr_len)) == -1) {
        if ((num_byte = recvfrom(sockfd, pkt_buf, sizeof(packet), 0, NULL, NULL)) == -1) {
            printf("[ERROR]: Failed to receive ACK from receiver\n");
            exit(2);
        }

        memcpy(&pkt, pkt_buf, sizeof(packet));
        if (pkt.msg_type == FIN_ACK) {
            printf("[INFO]: Receive FIN_ACK\n");
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

	// Determine how many bytes to transfer 

    slen = sizeof (si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    */

	/* Send data and receive acknowledgements on s*/
    
    bytesToRead = bytesToTransfer;
    int sockfd = get_socket(hostname, hostUDPport);

    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("[ERROR]: Could not open file %s\n", filename);
        exit(1);
    }
    unsigned long long int total_pkt = (unsigned long long int) ceil((float)bytesToTransfer / MSS);
    printf("[INFO]: %lld packet(s) need to be sent\n", total_pkt);

    fillBuffer(BUFFER_SIZE, fp);
    set_socket_timeout(sockfd);
    sendPkts(sockfd, fp);
    while (!buffer.empty() || !wait_ack.empty()) {
        if((numbytes = recvfrom(sockfd, pkt_buf, sizeof(packet), 0, NULL, NULL)) == -1) {
            if (errno != EAGAIN || errno != EWOULDBLOCK) {
                perror("can not receive main ack");
                exit(2);
            }
            printf("[INFO]: Time out, resend packet %d\n", wait_ack.front().seq_num);
            // cout << "Time out, resend pkt " << wait_ack.front().seq_num << endl;
            memcpy(pkt_buf, &wait_ack.front(), sizeof(packet));
            if((numbytes = sendto(sockfd, pkt_buf, sizeof(packet), 0, p->ai_addr, p->ai_addrlen))== -1){
                perror("Error: data sending");
                printf("[ERROR]: Fail to send packet %d\n", wait_ack.front().seq_num);
                exit(2);
            }
            congestionControl(false, true);
        } else {
            packet pkt;
            memcpy(&pkt, pkt_buf, sizeof(packet));
            if (pkt.msg_type == ACK) {
                printf("[INFO]: Receive ACK %d\n", pkt.ack_num);
                // cout << "receive ack" << pkt.ack_num << endl;
                if (pkt.ack_num == wait_ack.front().seq_num) {
                    congestionControl(false, false);
                    if (dupAckCount == 3) {
                        ssthread = cwnd/2.0;
                        cwnd = ssthread + 3;
                        dupAckCount = 0;
                        congetion_ctrl_state = FAST_RECOVERY;
                        printf("[INFO]: 3 duplicate tp FAST_RECOVERY, cwnd = %f\n",cwnd);
                        // cout << "3 duplicate tp FAST_RECOVERY, cwnd = " << cwnd <<endl;
                        // resend duplicated pkt
                        memcpy(pkt_buf, &wait_ack.front(), sizeof(packet));
                        if((numbytes = sendto(sockfd, pkt_buf, sizeof(packet), 0, p->ai_addr, p->ai_addrlen))== -1){
                            perror("Error: data sending");
                            printf("Fail to send %d pkt", wait_ack.front().seq_num);
                            exit(2);
                        }
                        printf("[INFO]: 3 duplicate ACKs, resend packet %d\n", wait_ack.front().seq_num);
                        // cout << "3 duplicate ACKs, resend pkt " << wait_ack.front().seq_num << endl;
                    }
                } else if (pkt.ack_num > wait_ack.front().seq_num) {
                    while (!wait_ack.empty() && wait_ack.front().seq_num < pkt.ack_num) {
                        congestionControl(true, false);
                        wait_ack.pop();
                    }
                    sendPkts(sockfd, fp);
                }
            }
        }
    }
    fclose(fp);
    
    fin_ack(sockfd);

    printf("[INFO]: Closing the socket\n");
    close(s);
    return;
}

/**
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;
    unsigned long long int numBytes;

    if (argc != 5) {
        fprintf(stderr, "[USAGE]: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);

    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);

    return (EXIT_SUCCESS);
}
