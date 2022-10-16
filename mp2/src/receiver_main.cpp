/* 
 * File:   receiver_main.c
 * Author: 
 *
 * Created on
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <errno.h>

#define TIMEOUT_SECONDS 1
#define MAXDATASIZE 1100 // max number of bytes we can get at once
#define CONTENT_BUFFER_SIZE 1200

struct Packet {
    char content[CONTENT_BUFFER_SIZE];
    int seq_num;
    int bytes_read;
};

struct ACK_MSG {
    int ack_num;
};

using namespace std;
struct timeval tv;
struct sockaddr_in si_me, si_other, from;
int sockfd, slen, n;
socklen_t fromlen;

void diep(char *s) {
    perror(s);
    exit(1);
}


string convertToString(char* chars, int size) {
    string s(chars);
    // if size x is passed in, we return its first x chars
    if(size != -1) {
        return s.substr(0, size);
    }
    return s;
}

void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {

    ofstream received_file;
    int numbytesReceived;

    slen = sizeof (si_other);
    fromlen  = sizeof(struct sockaddr_in);


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("Now binding\n");
    if (bind(sockfd, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("bind");

    cout << "[Receiver] - Receiving IP used: " << inet_ntoa(si_me.sin_addr) << endl;
    cout << "[Receiver] - Receiving port used: " << htons(si_me.sin_port) << endl;

    /* Set timeout for socket */
    tv.tv_sec = TIMEOUT_SECONDS * 3;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

    /* Now receive data and send acknowledgements */
    string destinationFileStr = destinationFile;
    received_file.open(destinationFileStr.c_str(), ios::binary);
    fromlen = sizeof(from);
    int waiting_seq_num = 0;

    while (true) {

        cout << "--------------------------------------------------------" << endl;
        Packet buf;
        numbytesReceived = ::recvfrom(sockfd, &buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen);


        if(errno == EAGAIN || errno == EWOULDBLOCK) {
            /*
             * All files are transmitted.
             * Sender Timeout!
             * Let's close the socket : )
             * */
            cout << "[Receiver] - Sender TIMEOUT! Socket closed! " << endl;
            break;
        }

        cout << "[Receiver] - current received seq_num: " << buf.seq_num << endl;
        cout << "[Receiver] - numbytesReceived: " << buf.bytes_read << endl;
//        cout << "[Receiver] - buf.content:{" << buf.content << "}" << endl;
        cout << "[Receiver] - current waiting_seq_num: " << waiting_seq_num << "" << endl;

        if(n < 0) {
            fprintf(stderr, "[Receiver] - recv: %s (%d)\n", strerror(errno), errno);
            cout << "[Receiver] - n: " << n << endl;
            continue;
        }
        if (numbytesReceived > 0) {
            ACK_MSG ack_msg;
            if(buf.seq_num == waiting_seq_num) {
                ack_msg.ack_num = buf.seq_num + 1;

                // mock packet loss
//                double ran = ((double) rand() / (RAND_MAX));
//                if(ran < 0.03) {
//                    cout << "********[Receiver]: ran: " << ran << " **********" << endl;
//                    cout << "********[Receiver]: PACKET LOSS OCCURRED FOR ACK: " << ack_msg.ack_num << " **********" << endl;
//                } else {
//                    n = ::sendto(sockfd, &ack_msg, sizeof(ack_msg), 0, (struct sockaddr *) &from, fromlen);
//                }

                n = ::sendto(sockfd, &ack_msg, sizeof(ack_msg), 0, (struct sockaddr *) &from, fromlen);
                received_file.write(buf.content, sizeof(char) * buf.bytes_read);
                waiting_seq_num += 1;
            } else {
                ack_msg.ack_num = waiting_seq_num;
                if( ((double) rand() / (RAND_MAX)) < 0.03) {
                    cout << "********[Receiver]: PACKET LOSS OCCURRED FOR ACK: " << ack_msg.ack_num << " **********" << endl;
                } else {
                    n = ::sendto(sockfd, &ack_msg, sizeof(ack_msg), 0, (struct sockaddr *) &from, fromlen);
                }
//                n = ::sendto(sockfd, &ack_msg, sizeof(ack_msg), 0, (struct sockaddr *) &from, fromlen);
                cout << "[Receiver] - ack " << ack_msg.ack_num << " sent!" << endl;
            }
            // else, drop the packet
        } else {
            fprintf(stderr, "[Receiver] - numbytesReceived < 0: %s (%d)\n", strerror(errno), errno);
        }
    }


    close(sockfd);
    printf("%s received.", destinationFile);
    return;

}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }


    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}

