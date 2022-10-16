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

#include "receiver_main.h"

void diep(char *s) {
    perror(s);
    exit(1);
}

void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {
    slen = sizeof (si_other);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("[INFO]: Now binding\n");
    if (bind(sockfd, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("bind");

    printf("[INFO]: Client IP   %s\n", inet_ntoa(si_me.sin_addr));
    printf("[INFO]: Client Port %d\n", htons(si_me.sin_port));

    /* set timeout for socket */
    struct timeval tv;
    tv.tv_sec = TIMEOUT*3;
    tv.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) == -1) {
        printf("[ERROR]: Failed to set socket timeout\n");
    }

    /* Now receive data and send acknowledgements */
    FILE *fp = fopen(destinationFile,"wb");
    int seq_num = 0;
    int byte_num;
    packet pkt;
    ACK ack;

    while(1) {
        printf("--------------------------------------------------\n");
        if((byte_num = recvfrom(sockfd, &pkt, sizeof(pkt), 0, (struct sockaddr *)&s_addr, &s_addrlen)) == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("[INFO] Server TIMEOUT Closing Socket\n");
                break;
            } else {
                printf("[ERROR] recvfrom fail\n");
            }
        }

        printf("[INFO]: seq_num %d\n", seq_num);
        printf("[INFO]: Receive packet %d, %d byte(s)\n", pkt.seq_num, pkt.size);
        
        if (byte_num > 0) {
            if(pkt.seq_num == seq_num) {
                ack.ack_num = pkt.seq_num+1;
                if(sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *) &s_addr, s_addrlen) == -1) {
                    printf("[ERROR]: Fail to send ACK to server\n");
                    exit(1);
                }
                printf("[INFO]: Send ACK %d successfully\n", ack.ack_num);
                fwrite(&pkt.data, sizeof(char), pkt.size, fp);
                seq_num += 1;
            } else {
                ack.ack_num = seq_num;
                if(sendto(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *) &s_addr, s_addrlen) == -1) {
                    printf("[ERROR]: Fail to send ACK to server\n");
                    exit(1);
                }
                printf("[INFO]: Send ACK %d successfully\n", ack.ack_num);
            }
        } else {
            printf("[INFO]: recvfrom receives byte <= 0\n");
        }
    }
    
    fclose(fp);
    close(sockfd);
    printf("[INFO]: File %s received successfully\n", destinationFile);
    return;
}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "[USAGE]: %s <UDP_port> <filename_to_write>\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}
