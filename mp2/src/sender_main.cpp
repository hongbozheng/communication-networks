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

using namespace std;

void diep(char *s) {
    perror(s);
    exit(1);
}

string convertToString(char* chars, int size) {
    string s(chars);
    // if size x is passed in, we return its first x chars
    if(size != -1) {
        return s.substr(size);
    }
    return s;
}

void printArr(int arr[], int size) {
    printf("[");
    for(int i = 0; i < size; ++i) {
        printf(" %d ",arr[i]);
    }
    printf(" ]\n");
}


void printWindow(const deque<packet> v){
    printf("[");
    for(unsigned i = 0; i < v.size(); ++i) {
        printf(" %d ",v[i].seq_num);
    }
    printf(" ]\n");

}

int get_lask_acked_seq(int arr[], int size) {
    for(int i = size-1; i >= 0; i--) {
        if(arr[i]) {
            return i;
        }
    }
    return -1;
}


int get_first_absent_seq(bool arr[]) {
    for(int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++) {
        if(!arr[i]) {
            return i;
        }
    }
    return -1;
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
    tv.tv_sec = TIMEOUT_SECONDS;
    tv.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) == -1) {
        printf("[ERROR]: Failed to set socket timeout\n");
    }

	/* Send data and receive acknowledgements on s */
    int cwnd = MSS;
    int ssthreash = INIT_SSTHRESH;
    int seq_num = 0;
    int byte_xfer_total = 0;
    int byte_num;
    bool final_packet_read = false;
    packet pkt;
    ACK ack;
    deque<packet> pkt_q;
    unordered_map<int, int> ack_freq_map;

    while(1) {
        if(final_packet_read && pkt_q.size() == 0) {
            break;
        }

        printf("--------------------------------------------------\n");
        int pkt_num = cwnd / MSS;
        printf("[INFO]: cwnd      %d\n", cwnd);
        printf("[INFO]: ssthreash %d\n", ssthreash);
        printf("[INFO]: MSS       %d\n", MSS);
        bool pkt_timeout = false;
        bool ack_3 = false;

        while(pkt_q.size() < pkt_num) {
            if(byte_xfer_total >= bytesToTransfer) {
                final_packet_read = true;
                break;
            }
            else if(byte_xfer_total + BUFFER_SIZE > bytesToTransfer) {
                byte_num = fread(pkt.data, sizeof(char), bytesToTransfer - byte_xfer_total, fp);
            } else {
                byte_num = fread(pkt.data, sizeof(char), BUFFER_SIZE, fp);
            }

            if(byte_num == 0) {
                final_packet_read = true;
                break;
            }

            byte_xfer_total += byte_num;
            pkt.seq_num = seq_num++;
            pkt.size = byte_num;
            pkt_q.push_back(pkt);
        }

        int seq_start = pkt_q.front().seq_num;
        int seq_end = pkt_q.back().seq_num;
        printf("[INFO]: cwnd seq num %d ~ %d\n", seq_start, seq_end);
        int ACK_receive[seq_end-seq_start+1];
        fill(ACK_receive, ACK_receive+ seq_end-seq_start+1, 0);

        printf("[INFO]: packets to be sent this round: ");
        printWindow(pkt_q);
        for(int i = 0; i < pkt_q.size(); ++i) {
            if (sendto(sockfd, &pkt_q[i], sizeof(pkt_q[i]), 0, (struct sockaddr *) &si_other, sizeof(si_other)) == -1) {
                printf("[ERROR]: Fail to send packet to client\n");
                exit(1);
            }
            printf("[INFO]: Packet seq_num %d is sent successfully\n", pkt_q[i].seq_num);
        }

        auto start_time = std::chrono::system_clock::now();
        for(int i = 0; i < pkt_q.size(); ++i) {
            if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start_time).count() >= TIMEOUT_SECONDS) {
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

        int lask_acked_seq_index = get_lask_acked_seq(ACK_receive, sizeof(ACK_receive) / sizeof(ACK_receive[0]));
        cout << "[sender]: seq_received this round: " << endl;
        printArr(ACK_receive, sizeof(ACK_receive) / sizeof(ACK_receive[0]));
        cout << "[sender]: lask_acked_seq_index: " << lask_acked_seq_index << endl;
        if(lask_acked_seq_index != -1) {
            for(int i = 0; i <= lask_acked_seq_index; ++i) {
                pkt_q.pop_front();
            }
        }

        if(pkt_timeout) {
            ssthreash = cwnd / 2;
            cwnd = MSS;
            printf("[INFO]: ACK %d TIMEOUT\n", ack.seq_num);
            printf("[INFO]: cwnd      %d", cwnd);
            printf("[INFO]: ssthreash %d", ssthreash);
            printf("[INFO]: %d", (cwnd / MSS));
        } else if(ack_3) {
            ssthreash = cwnd / 2;
            cwnd = ssthreash + 3 * MSS;
            printf("[INFO]: 3 duplicate ACK %d", ack.seq_num);
            cout << "[sender]: current ssthreash: " << ssthreash << endl;
            cout << "[sender]: current cwnd: " << (cwnd / MSS) << endl;
        } else {
            if(cwnd >= ssthreash) {
                cwnd += MSS;
                cout << "[sender]: Congestion Control Phase" << endl;
                cout << "[sender]: current ssthreash: " << ssthreash << endl;
                cout << "[sender]: current cwnd: " << (cwnd / MSS) << endl;
            } else {
                cout << "[sender]: Slow Start Phase" << endl;
                if(cwnd * 2 >= ssthreash) {
                    cwnd = ssthreash;
                } else {
                    cwnd *= 2;
                }
                cout << "[sender]: current ssthreash: " << ssthreash << endl;
                cout << "[sender]: current cwnd: " << (cwnd / MSS) << endl;
            }
        }

        ack_freq_map.clear();
    }

    printf("[INFO]: All packet sent successfully %d\n", ssthreash);
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
