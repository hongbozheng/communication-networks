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

    char file_buffer [BUFF_SIZE];

    int nextACK = 0;
    int already_ACK [TOTAL_CNT];
    int data_size [TOTAL_CNT];
    for (int i=0; i<TOTAL_CNT; i++){
        already_ACK[i]=0;
        data_size[i]=DATA_SIZE;
    }
    addrlen = sizeof sender_addr;
    int cur_idx = 0;
    while(1){
        recv_byte = recvfrom(s, buf, sizeof(packet), 0, (struct sockaddr*)&sender_addr, &addrlen);
        if (recv_byte <= 0) {
            fprintf(stderr, "Connection closed\n");
            exit(2);
        }
        packet pkt;
        memcpy(&pkt,buf,sizeof(packet));
        printf("[INFO]: Receive packet %d, Type %d\n", pkt.seq_num, pkt.msg_type);
        //cout << "receive pkt" << pkt.seq_num<< " type " << pkt.msg_type<< endl;
        if (pkt.msg_type==DATA){
            if(pkt.seq_num==nextACK){
                memcpy(&file_buffer[cur_idx*DATA_SIZE], &pkt.data , pkt.data_size);
                fwrite(&file_buffer[cur_idx*DATA_SIZE],sizeof(char),pkt.data_size,fp);
                printf("[INFO]: Write packet %d\n", pkt.seq_num);
                //cout << "write pkt "<< pkt.seq_num << endl;
                nextACK++;
                cur_idx  = (cur_idx+1)%TOTAL_CNT;
                while(already_ACK[cur_idx]==1){
                    fwrite(&file_buffer[cur_idx*DATA_SIZE],sizeof(char),data_size[cur_idx],fp);
                    //cout << "write index "<< cur_idx << endl;
                    printf("[INFO]: Write index %d\n", cur_idx);
                    already_ACK[cur_idx] = 0;
                    cur_idx  = (cur_idx+1)%TOTAL_CNT;
                    nextACK++;
                }
            }else if(pkt.seq_num>nextACK){
                int ahead_idx = (cur_idx+pkt.seq_num-nextACK)%TOTAL_CNT;
                for (int i=0;i<pkt.data_size;i++) {
                    file_buffer[ahead_idx*DATA_SIZE+i] = pkt.data[i];
                }
                already_ACK[ahead_idx] = 1;
                data_size[ahead_idx] = pkt.data_size;
            }
            packet ack;
            ack.msg_type=ACK;
            ack.ack_num = nextACK;
            ack.data_size = 0; // data size is 0 since we are sending ack
            memcpy(buf,&ack,sizeof(packet));
            sendto(s, buf, sizeof(packet), 0, (struct sockaddr *) &sender_addr,addrlen);
            printf("[INFO]: Sent ACK %d\n", ack.ack_num);
            //cout << "sent ack" << ack.ack_num << endl;
        }else if(pkt.msg_type== FIN){
            //fwrite(file_buffer,sizeof(char),BUFF_SIZE,fp);
            packet ack;
            ack.msg_type=FIN_ACK;
            ack.ack_num = nextACK;
            ack.data_size = 0;
            memcpy(buf,&ack,sizeof(packet));
            sendto(s, buf, sizeof(packet), 0, (struct sockaddr *) &sender_addr,addrlen);
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

