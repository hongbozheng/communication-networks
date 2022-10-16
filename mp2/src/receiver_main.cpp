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
    tv.tv_sec = TIMEOUT_SECONDS*3;
    tv.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) == -1) {
        printf("[ERROR]: Failed to set socket timeout\n");
    }

    /* Now receive data and send acknowledgements */
    ofstream received_file;
    string destinationFileStr = destinationFile;
    received_file.open(destinationFileStr.c_str(), ios::binary);
    int waiting_seq_num = 0;
    int byte_num;

    while (true) {

        cout << "--------------------------------------------------------" << endl;
        Packet buf;
        byte_num = ::recvfrom(sockfd, &buf, sizeof(buf), 0, (struct sockaddr *)&s_addr, &s_addrlen);


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
        if (byte_num > 0) {
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

                n = ::sendto(sockfd, &ack_msg, sizeof(ack_msg), 0, (struct sockaddr *) &s_addr, s_addrlen);
                received_file.write(buf.content, sizeof(char) * buf.bytes_read);
                waiting_seq_num += 1;
            } else {
                ack_msg.ack_num = waiting_seq_num;
                if( ((double) rand() / (RAND_MAX)) < 0.03) {
                    cout << "********[Receiver]: PACKET LOSS OCCURRED FOR ACK: " << ack_msg.ack_num << " **********" << endl;
                } else {
                    n = ::sendto(sockfd, &ack_msg, sizeof(ack_msg), 0, (struct sockaddr *) &s_addr, s_addrlen);
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
