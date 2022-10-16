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

#include "sender_main.h"

void diep(char *s) {
    perror(s);
    exit(1);
}

int get_last_ack_seq_num(int arr[], int size) {
    for(int i = size-1; i >= 0; i--) {
        if(arr[i]) {
            return i;
        }
    }
    return -1;
}

void state_ctrl(bool pkt_timeout, bool ack_3, ACK ack) {
    if(pkt_timeout) {
        ssthreash = cwnd_byte/2;
        cwnd_byte = MSS;
        printf("[INFO]: ACK %d TIMEOUT\n", ack.seq_num);
        //printf("[INFO]: cwnd      %d\n", cwnd_byte/MSS);
        //printf("[INFO]: cwnd_byte %d\n", cwnd_byte);
        //printf("[INFO]: ssthreash %d\n", ssthreash);
    } else if(ack_3) {
        ssthreash = cwnd_byte/2;
        cwnd_byte = ssthreash + 3*MSS;
        printf("[INFO]: 3 duplicate ACK %d\n", ack.seq_num);
        //printf("[INFO]: cwnd      %d\n", cwnd_byte/MSS);
        //printf("[INFO]: cwnd_byte %d\n", cwnd_byte);
        //printf("[INFO]: ssthreash %d\n", ssthreash);
    } else {
        if(cwnd_byte >= ssthreash) {
            cwnd_byte += MSS;
            printf("[INFO]: Congestion Control Phase\n");
            //printf("[INFO]: cwnd      %d\n", cwnd_byte/MSS);
            //printf("[INFO]: cwnd_byte %d\n", cwnd_byte);
            //printf("[INFO]: ssthreash %d\n", ssthreash);
        } else {
            printf("[INFO]: Slow Start Phase\n");
            if(cwnd_byte * 2 >= ssthreash) {
                cwnd_byte = ssthreash;
            } else {
                cwnd_byte *= 2;
            }
            //printf("[INFO]: cwnd      %d\n", cwnd_byte/MSS);
            //printf("[INFO]: cwnd_byte %d\n", cwnd_byte);
            //printf("[INFO]: ssthreash %d\n", ssthreash);
        }
    }
}

void reliablyTransfer(char* hostname, unsigned short int hostUDPport, char* filename, unsigned long long int bytesToTransfer) {
    
    FILE *fp;
    fp = fopen(filename, "rb");
    printf("[INFO]: Open file %s\n",filename);
    if (fp == NULL) {
        printf("[ERROR]: Could not open file %s to send\n",filename);
        exit(1);
    }

	/* Determine how many bytes to transfer */

    slen = sizeof (si_other);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        diep("socket");
    }

    memset((char *) &si_other, 0, sizeof (si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(hostUDPport);
    if (inet_aton(hostname, &si_other.sin_addr) == 0) {
        printf("[ERROR]: inet_aton() failed\n");
        exit(1);
    }

    /* set timeout for socket */
    struct timeval tv;
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) == -1) {
        printf("[ERROR]: Failed to set socket timeout\n");
    }

	/* Send data and receive acknowledgements on s */
    int seq_num = 0;
    int byte_xfer_total = 0;
    int byte_num;
    bool xfer_fin = false;
    packet pkt;
    ACK ack;
    std::deque<packet> pkt_q;
    std::unordered_map<int, int> ack_freq_map;

    while(1) {
        printf("--------------------------------------------------\n");
        int pkt_num_max = cwnd_byte/MSS;
        printf("[INFO]: cwnd      %d\n", cwnd_byte/MSS);
        printf("[INFO]: cwnd_byte %d\n", cwnd_byte);
        printf("[INFO]: MSS       %d\n", MSS);
        printf("[INFO]: ssthreash %d\n", ssthreash);
        bool pkt_timeout = false;
        bool ack_3 = false;

        while((pkt_q.size() < pkt_num_max)) {
            if(byte_xfer_total >= bytesToTransfer){
                xfer_fin = true;
                break;
            }

            if(byte_xfer_total + BUFFER_SIZE > bytesToTransfer) {
                byte_num = fread(pkt.data, sizeof(char), bytesToTransfer - byte_xfer_total, fp);
            } else {
                byte_num = fread(pkt.data, sizeof(char), BUFFER_SIZE, fp);
            }

            if(byte_num == 0) {
                xfer_fin = true;
                break;
            }

            byte_xfer_total += byte_num;
            pkt.seq_num = seq_num++;
            pkt.size = byte_num;
            pkt_q.push_back(pkt);
        }

        if(xfer_fin && pkt_q.size() == 0) {
            break;
        }

        int seq_start = pkt_q.front().seq_num;
        int seq_end = pkt_q.back().seq_num;
        printf("[INFO]: cwnd seq num %d ~ %d\n", seq_start, seq_end);
        int ACK_receive[seq_end-seq_start+1];
        std::fill(ACK_receive, ACK_receive+ seq_end-seq_start+1, 0);

        for(int i = 0; i < pkt_q.size(); ++i) {
            if (sendto(sockfd, &pkt_q[i], sizeof(pkt_q[i]), 0, (struct sockaddr *) &si_other, sizeof(si_other)) == -1) {
                printf("[ERROR]: Fail to send packet %d to client\n", pkt_q[i].seq_num);
                exit(1);
            }
            printf("[INFO]: Packet seq_num %d is sent successfully\n", pkt_q[i].seq_num);
        }

        auto start_time = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < pkt_q.size(); ++i) {
            if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - start_time).count() >= TIMEOUT) {
                printf("[INFO]: WAITING ACK TIMEOUT\n");
                break;
            }

            if((byte_num = recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&c_addr, &c_addrlen)) == -1) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    printf("[INFO]: Fail to receive ACK\n");
                    pkt_timeout = true;
                } else {
                    printf("[ERROR]: recvfrom fail\n");
                    exit(1);
                }
            } else {
                printf("[INFO]: ACK RECEIVED %d\n", ack.seq_num);
                ACK_receive[ack.seq_num-seq_start-1] = 1;
                if(ack_freq_map.count(ack.seq_num) == 0) {
                    ack_freq_map[ack.seq_num] = 1;
                } else {
                    ack_freq_map[ack.seq_num] += 1;
                    if(ack_freq_map[ack.seq_num] == 3) {
                        ack_3 = true;
                    }
                }
            }
        }

        int last_ACK_seq_num = get_last_ack_seq_num(ACK_receive, sizeof ACK_receive/sizeof ACK_receive[0]);
        printf("[INFO]: Last ACK seq num %d, Starting removing from pkt_q\n", last_ACK_seq_num);
        if(last_ACK_seq_num != -1) {
            for(int i = 0; i <= last_ACK_seq_num; ++i) {
                pkt_q.pop_front();
            }
        }

        state_ctrl(pkt_timeout, ack_3, ack);
        ack_freq_map.clear();
    }
    fclose(fp);
   
    printf("--------------------------------------------------\n");
    while(1) {
        pkt.seq_num = -1;
        pkt.size = 0;
        if(sendto(sockfd, &pkt, sizeof(packet), 0, (struct sockaddr *) &si_other, sizeof (si_other)) == -1) {
            printf("[ERROR]: Failed to send FIN to client\n");
            exit(1);
        }
        printf("[INFO]: Send FIN to client\n");
        if(recvfrom(sockfd, &ack, sizeof(ack), 0, (struct sockaddr *)&c_addr, &c_addrlen) == -1) {
            if(errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("[INFO]: Fail to receive FIN ACK\n");
                break;
            } else {
                printf("[ERROR]: recvfrom fail\n");
                exit(1);
            }
        } else {
                printf("[INFO]: ACK RECEIVED %d\n", ack.seq_num);
        }
        if(ack.seq_num == -1) {
            printf("[INFO]: Receive the FIN_ACK\n");
            break;
        }
    }

    printf("[INFO]: All packet(s) sent successfully\n");
    printf("[INFO]: Closing the socket\n");
    close(sockfd);
    return;
}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;
    unsigned long long int numBytes;

    if (argc != 5) {
        fprintf(stderr, "[USAGE]: %s <receiver_hostname> <receiver_port> <filename_to_xfer> <bytes_to_xfer>\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);

    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);

    return (EXIT_SUCCESS);
}
