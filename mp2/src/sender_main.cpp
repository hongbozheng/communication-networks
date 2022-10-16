/* 
 * File:   sender_main.c
 * Author: 
 *
 * Created on 
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

void printArr(bool arr[], int size) {
    printf("[");
    for(int i = 0; i < size; ++i) {
        printf(" %d ",arr[i]);
    }
    printf(" ]\n");
}


void printWindow(const deque<Packet> v){
    printf("[");
    for(unsigned i = 0; i < v.size(); ++i) {
        printf(" %d ",v[i].seq_num);
    }
    printf(" ]\n");

}

int get_lask_acked_seq(bool arr[], int size) {
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

    /* Set timeout for socket */
    struct timeval tv;
    tv.tv_sec = TIMEOUT_SECONDS;
    tv.tv_usec = 0;
    if(setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) == -1) {
        printf("[ERROR]: Failed to set socket timeout\n");
    }

	/* Send data and receive acknowledgements on s */
    c_addrlen= sizeof(c_addr);
    transferredBytes = 0;
    int MSS = sizeof(char) * BUFFER_SIZE;
    int cwnd_size = MSS;
    deque<Packet> cwnd;
    int current_bytes_read = 0;
    int cumulative_bytes_read = 0;
    ssthreash = INIT_SSTHRESH;
    int start_offset = 0;
    int i;
    int seq_num = SEQUENCE_NUM_INIT;
    struct ACK_MSG ack_msg;
    unordered_map<int, int> ack_freq_map;
    bool final_packet_read = false;
//    double r = ((double) rand() / (RAND_MAX));

    while(true) {
        cout << "--------------------------------------------------------" << endl;
        /* Read bytes from the file and copy it to the current window */
        int cwnd_capacity = cwnd_size / MSS;
        cout << "[Sender]: new round of cwnd_size: " << cwnd_size << endl;
        cout << "[Sender]: new round of ssthreash: " << ssthreash << endl;
        cout << "[Sender]: new round of MSS: " << MSS << endl;
        cout << "[Sender]: new round of cwnd: " << endl;
        cout << "[Sender]: new round of start_offset: " << start_offset << endl;

        dupACKcount = 0;
        bool has_timeout_packet = false;
        bool has_3_dup_ack = false;
        while(cwnd.size() < cwnd_capacity) {
            Packet packet = Packet();

            if(cumulative_bytes_read >= bytesToTransfer) {
                final_packet_read = true;
                break;
            }

            if(cumulative_bytes_read + BUFFER_SIZE > bytesToTransfer) {
                current_bytes_read = fread(packet.content, sizeof(char), bytesToTransfer - cumulative_bytes_read, fp);
                cout << "[Sender]: A - current bytes read: " << current_bytes_read << endl;
            } else {
                current_bytes_read = fread(packet.content, sizeof(char), BUFFER_SIZE, fp);
                cout << "[Sender]: B - current bytes read: " << current_bytes_read << endl;
            }

//            cout << "[Sender]: A - current content read: " << packet.content << endl;

            if(current_bytes_read == 0) {
                final_packet_read = true;
                break;
            }

            cumulative_bytes_read += current_bytes_read;
            packet.seq_num = seq_num++;
            packet.bytes_read = current_bytes_read;
//            cout << "[Sender]: current packet size: " << sizeof(packet) << endl;

            cwnd.push_back(packet);
        }

        if(final_packet_read && cwnd.size() == 0) {
            break;
        }

        int start_seq_this_round = cwnd.front().seq_num;
        int end_seq_this_round = cwnd.back().seq_num;
        cout << "[Sender]: start_seq_this_round: " << start_seq_this_round << endl;
//        cout << "[Sender]: end_seq_this_round: " << end_seq_this_round << endl;
//        cout << "[Sender]: seq_received size: " << end_seq_this_round-start_seq_this_round+1 << endl;

        bool seq_received[end_seq_this_round-start_seq_this_round+1];
//        cout << "[Sender]: init seq_received: " << end_seq_this_round << endl;
//        cout << "[Sender]: init seq_received size: " << sizeof(seq_received) / sizeof(seq_received[0]) << endl;
//        printArr(seq_received, sizeof(seq_received) / sizeof(seq_received[0]));
        fill(seq_received, seq_received + end_seq_this_round-start_seq_this_round+1, false);
//        cout << "[Sender]: after seq_received: " << end_seq_this_round << endl;
//        printArr(seq_received, sizeof(seq_received) / sizeof(seq_received[0]));

        /* Sending file to the receiver */

        cout << "[Sender]: packets to be sent this round: " << endl;
        printWindow(cwnd);


        for(int j = 0; j < cwnd.size(); j++) {

            // mock packet loss
//            if( ((double) rand() / (RAND_MAX)) < 0.05) {
//                cout << "********[Sender]: PACKET LOSS OCCURRED FOR seq_num: " << cwnd[j].seq_num << " **********" << endl;
//                continue;
//            }

            if (sendto(sockfd, &cwnd[j], sizeof(cwnd[j]), 0, (struct sockaddr *) &si_other, sizeof(si_other)) == -1) {
                perror("send");
                exit(1);
            }


            cout << "[Sender]: packet with seq_num " << cwnd[j].seq_num << " is sent!" << endl;
        }

        /* Waiting for the ACK */
        auto start = std::chrono::system_clock::now();
        for(int j = 0; j < cwnd.size(); j++) {
            /* Timeout the for loop when the time is out */
            auto end = std::chrono::system_clock::now();
            if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start).count() >= TIMEOUT_SECONDS) {
                cout << "[sender]: this round of ack waiting is timeout! " << endl;
                break;
            }

            /* Receiving ACKs */
            n = recvfrom(sockfd, &ack_msg, sizeof(ack_msg), 0, (struct sockaddr *)&c_addr, &c_addrlen);
            if(n < 0) {
                if(errno == EAGAIN || errno == EWOULDBLOCK) {
                    // Receiving ACK TIMEOUT. Retransmit...
                    cout << "[sender]: ack is missing.. resending... " << ack_buffer << endl;
                    has_timeout_packet = true;
                } else {
                    // Other errors occurred. Error msg printed.
                    fprintf(stderr, "[Receiver] - recv: %s (%d)\n", strerror(errno), errno);
                    diep("[sender] - recvfrom receiver error..");
                }
            } else {
                cout << "[sender]: ACK RECEIVED: " << ack_msg.ack_num << endl;
                seq_received[ack_msg.ack_num-1-start_seq_this_round] = true;
                if(ack_freq_map.count(ack_msg.ack_num) == 0) {
                    ack_freq_map[ack_msg.ack_num] = 1;
                } else {
                    ack_freq_map[ack_msg.ack_num] += 1;
                    if(ack_freq_map[ack_msg.ack_num] == 3) {
                        has_3_dup_ack = true;
                    }
                }
            }
        }

        /* Prep for the next round */
//        int first_absent_seq_index = get_first_absent_seq(seq_received);
//        cout << "[sender]: seq_received this round: " << endl;
//        printArr(seq_received, sizeof(seq_received) / sizeof(seq_received[0]));
//        cout << "[sender]: first_absent_seq_index: " << first_absent_seq_index << endl;
//        if(first_absent_seq_index != -1) {
//            for(int j = 0; j < first_absent_seq_index; j++) {
//                cwnd.pop_front();
//            }
//        } else {
//            // no packet loss this round
//            cwnd.clear();
//        }

        int lask_acked_seq_index = get_lask_acked_seq(seq_received, sizeof(seq_received) / sizeof(seq_received[0]));
        cout << "[sender]: seq_received this round: " << endl;
        printArr(seq_received, sizeof(seq_received) / sizeof(seq_received[0]));
        cout << "[sender]: lask_acked_seq_index: " << lask_acked_seq_index << endl;
        if(lask_acked_seq_index != -1) {
            for(int j = 0; j <= lask_acked_seq_index; j++) {
                cwnd.pop_front();
            }
        }

        if(has_timeout_packet) {
            ssthreash = cwnd_size / 2;
            cwnd_size = MSS;
            cout << "[sender]: This round has timeout packet: " << ack_msg.ack_num << endl;
            cout << "[sender]: current ssthreash: " << ssthreash << endl;
            cout << "[sender]: current MSS: " << MSS << endl;
            cout << "[sender]: current cwnd_size: " << cwnd_size << endl;
            cout << "[sender]: current test: " << (1000/1000) << endl;
            cout << "[sender]: current cwnd window size: " << (cwnd_size / MSS) << endl;
        } else if(has_3_dup_ack) {
            ssthreash = cwnd_size / 2;
            cwnd_size = ssthreash + 3 * MSS;
            cout << "[sender]: This round has 3 duplicate acks: " << ack_msg.ack_num << endl;
            cout << "[sender]: current ssthreash: " << ssthreash << endl;
            cout << "[sender]: current cwnd: " << (cwnd_size / MSS) << endl;
        } else {
            if(cwnd_size >= ssthreash) {
                cwnd_size += MSS;
                cout << "[sender]: Congestion Control Phase" << endl;
                cout << "[sender]: current ssthreash: " << ssthreash << endl;
                cout << "[sender]: current cwnd: " << (cwnd_size / MSS) << endl;
            } else {
                cout << "[sender]: Slow Start Phase" << endl;
                if(cwnd_size * 2 >= ssthreash) {
                    cwnd_size = ssthreash;
                } else {
                    cwnd_size *= 2;
                }
                cout << "[sender]: current ssthreash: " << ssthreash << endl;
                cout << "[sender]: current cwnd: " << (cwnd_size / MSS) << endl;
            }
        }

        ack_freq_map.clear();
    }

    cout << "[sender]: All packet sent... " << ssthreash << endl;
    printf("[sender]: Closing the socket\n");
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
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }
    udpPort = (unsigned short int) atoi(argv[2]);
    numBytes = atoll(argv[4]);

    reliablyTransfer(argv[1], udpPort, argv[3], numBytes);


    return (EXIT_SUCCESS);
}


