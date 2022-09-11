/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <assert.h>

#include <arpa/inet.h>

#define LISTENPORT "4950"	// the port users will be connecting to

#define MAXBUFLEN 100

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 500 // max number of bytes we can get at once 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char recBuf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
    static char buf[1000];


	// from listener.c

	//struct sockaddr_storage their_addr;
	//socklen_t addr_len;

	//int sockfd_listener, numbytes_listener;  
	//char buf_listener[MAXDATASIZE];
	//struct addrinfo hints_listener, *servinfo_listener, *p_listener;
	//int rv_listener;
	//char s_listener[INET6_ADDRSTRLEN];



	if (argc != 2) {
	    fprintf(stderr,"usage: http hostname\n");
	    exit(1);
	}
    
    //length of url
    int urlLen = strlen(argv[1]);
    // int urlType = 0;//0:"http://hostname[:port]/path/to/file", 1:"hostname[:port]/path/to/file"
    int slashCount = 0;
    int colonCount = 0;
    int hostStart = 7;
    int hostEnd = 0;
    int portStart = 0;
    int portEnd = 0;
    int portExist = 0;
    if(argv[1][0] != 'h') {
        fprintf(stderr,"usage: http_client http://hostname[:port]/path/to/file \n");
	    exit(1);
    }
    for(int i = 0; i < urlLen; i++){
        if(argv[1][i] == '/') {
            slashCount++;
            if(slashCount == 3){
                hostEnd = i - 1;
                if(portExist == 1){
                    portEnd = i - 1;
                }
            }
        }        
        if(argv[1][i] == ':') {
            colonCount++;
            if(colonCount == 2){
                portExist = 1;
                portStart = i + 1;
            }
        }

    }

	char host[hostEnd-hostStart+1];
    char fileDir[urlLen-hostEnd+1];
    char port[5];
    strncpy(host, argv[1]+hostStart, hostEnd-hostStart+1);
    strncpy(fileDir, argv[1]+hostEnd+1, urlLen-hostEnd+1);
	host[hostEnd-hostStart+1] = '\0';
    if(portExist == 1){
        strncpy(port, argv[1]+portStart, portEnd-portStart+1);
    }else{
        port[0] = '8';
		port[1] = '0';
		port[2] = '\0';
    }

    printf("host:%s\n", host);
    printf("fileir:%s\n", fileDir);
    printf("port:%s\n", port);

    sprintf(buf, "GET ");
    strcat(buf, fileDir);
    strcat(buf, " HTTP/1.1\r\n");
    strcat(buf, "User-Agent: Wget/1.12 (linux-gnu)\r\n");
    strcat(buf, "Host: ");
    strcat(buf, host);
    strcat(buf, "\r\n");
    strcat(buf, "Connection: Keep-Alive\r\n\r\n");
    
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		//printf("%d\n",sockfd);l
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

    send(sockfd, buf, sizeof(buf), 0);

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

    FILE *fp ;
    fp = fopen("output", "w");
    while(1){
        if ((numbytes = recv(sockfd, recBuf, MAXDATASIZE-1, 0)) > 0) {
            fprintf(fp, "%s", recBuf);
	        printf("num in line: % d\n", numbytes);
	    }
        else {
            fclose(fp);
            break;
        }
    }

	// char * pos;
	// pos=strchr(buf,'\n');
	// //printf("found at %zu\n",pos-buf+1);	
	// unsigned int str_position = pos-buf;
	// //printf("%zu \n\n",str_position);
	// printf("client: received %.*s bytes",str_position,buf);
	// printf("%s",buf+str_position+2);


	close(sockfd);

	// Start of listener Code

	//memset(&hints_listener, 0, sizeof hints_listener);
	//hints_listener.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	//hints_listener.ai_socktype = SOCK_DGRAM;
	//hints_listener.ai_flags = AI_PASSIVE; // use my IP

//	if ((rv_listener = getaddrinfo(NULL, LISTENPORT, &hints_listener, &servinfo_listener)) != 0) {
//		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));/
//		return 1;
//	}

	
	// loop through all the results and bind to the first we can
//	for(p_listener = servinfo_listener; p_listener != NULL; p_listener = p_listener->ai_next) {
//		if ((sockfd_listener = socket(p_listener->ai_family, p_listener->ai_socktype,
//				p_listener->ai_protocol)) == -1) {
//			perror("listener: socket");
//			continue;
//		}

//		if (bind(sockfd_listener, p_listener->ai_addr, p_listener->ai_addrlen) == -1) {
//			close(sockfd_listener);
//			perror("listener: bind");
//			continue;
//		}

//		break;
//	}

//	if (p_listener == NULL) {
//		fprintf(stderr, "listener: failed to bind socket\n");
//		return 2;
//	}

//	freeaddrinfo(servinfo_listener);

//	printf("listener: waiting to recvfrom...\n");

//	addr_len = sizeof their_addr;

//	if ((numbytes_listener = recvfrom(sockfd_listener, buf_listener, MAXBUFLEN-1 , 0,
//		(struct sockaddr *)&their_addr, &addr_len)) == -1) {
//		perror("recvfrom");
//		exit(1);
//	}

//	printf("listener: got packet from %s\n",
//		inet_ntop(their_addr.ss_family,
//			get_in_addr((struct sockaddr *)&their_addr),
//			s_listener, sizeof s_listener));
//	printf("listener: packet is %d bytes long\n", numbytes_listener);
//	buf_listener[numbytes_listener] = '\0';
//	printf("listener: packet contains \"%s\"\n", buf_listener);

//	close(sockfd_listener);



	return 0;
}

