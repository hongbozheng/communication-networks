/**
 * FILENAME: receiver_main.c
 * 
 * DESCRIPTION:
 *
 * DATE: Saturday, Oct 8th, 2022
 *
 * AUTHOR:
 *
 */

#include "receiver_main.h"

//struct sockaddr_in si_me, si_other;
//int s, slen;

void diep(char *s) {
    perror(s);
    exit(1);
}

void reliablyReceive(unsigned short int myUDPport, char* destinationFile) {
    
    slen = sizeof (si_other);

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        diep("socket");

    memset((char *) &si_me, 0, sizeof (si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(myUDPport);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    printf("[INFO]: Now binding\n");
    if (bind(s, (struct sockaddr*) &si_me, sizeof (si_me)) == -1)
        diep("[ERROR]: bind");

	/* Now receive data and send acknowledgements */ 
    FILE* fp = fopen(destinationFile,"wb");

    char file_buf [BUF_SIZE];

    int nextACK = 0;
    int ack[PKT_BUF_SIZE];
    int data_size [PKT_BUF_SIZE];
    for (int i = 0; i < PKT_BUF_SIZE; ++i){
        ack[i]=0;
        data_size[i]=DATA_SIZE;
    }

    addrlen = sizeof sender_addr;
    int idx = 0;
    
    packet pkt;

    while(1){
        recv_byte = recvfrom(s, buf, sizeof(packet), 0, (struct sockaddr*)&sender_addr, &addrlen);
        if (recv_byte <= 0) {
            fprintf(stderr, "Connection closed\n");
            exit(2);
        }
        memcpy(&pkt,buf,sizeof(packet));
        printf("[INFO]: Receive packet %d, Type %d\n", pkt.seq_num, pkt.msg_type);
        
        if (pkt.msg_type == DATA){
            if(pkt.seq_num == nextACK){
                memcpy(&file_buf[idx*DATA_SIZE], &pkt.data , pkt.data_size);
                fwrite(&file_buf[idx*DATA_SIZE],sizeof(char),pkt.data_size,fp);
                printf("[INFO]: Write packet %d\n", pkt.seq_num);
                nextACK++;
                idx  = (idx+1)%PKT_BUF_SIZE;
                while(ack[idx]==1){
                    fwrite(&file_buf[idx*DATA_SIZE],sizeof(char),data_size[idx],fp);
                    printf("[INFO]: Write index %d\n", idx);
                    ack[idx] = 0;
                    idx  = (idx+1)%PKT_BUF_SIZE;
                    nextACK++;
                }
            }else if(pkt.seq_num>nextACK){
                int ahead_idx = (idx+pkt.seq_num-nextACK)%PKT_BUF_SIZE;
                for (int i=0;i<pkt.data_size;i++) {
                    file_buf[ahead_idx*DATA_SIZE+i] = pkt.data[i];
                }
                ack[ahead_idx] = 1;
                data_size[ahead_idx] = pkt.data_size;
            }
            pkt.data_size = 0;
            pkt.msg_type = ACK;
            pkt.ack_num = nextACK;
            memcpy(buf, &pkt, sizeof(packet));
            sendto(s, buf, sizeof(packet), 0, (struct sockaddr *)&sender_addr, addrlen);
            printf("[INFO]: Sent ACK %d\n", pkt.ack_num);
        
        } else if(pkt.msg_type == FIN){
            pkt.data_size = 0;
            pkt.msg_type = FIN_ACK;
            pkt.ack_num = nextACK;
            memcpy(buf, &pkt, sizeof(packet));
            sendto(s, buf, sizeof(packet), 0, (struct sockaddr *)&sender_addr, addrlen);
            break;
        }
    }

    close(s);
	printf("[INFO]: File %s received\n", destinationFile);
    return;
}

/*
 * 
 */
int main(int argc, char** argv) {

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "[USAGE]: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    reliablyReceive(udpPort, argv[2]);
}

