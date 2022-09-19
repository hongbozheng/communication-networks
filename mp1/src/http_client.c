/**
 * FILENAME: http_client.c
 * 
 * DESCRIPTION: a http client application
 *              Supports multiple simultaneous http GET requests
 *              responding with 200 OK, 404 NOT FOUND, or 400 BAD REQUEST
 *
 * DATE: Saturday, Sep 17th, 2022
 *
 * AUTHOR:
 *
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
#include <arpa/inet.h>

#define MAXDATASIZE 1024 // max number of bytes we can get at once

// function signatures
void *get_in_addr(struct sockaddr *sa);
int sendall(int s, char *buf, int *len);
int sock_getline(int sock, char *buf, int size);
int main(int argc, char *argv[]);
void process_input(const char *input, char *host, char *port, char *path);
int connect_to_server(const char *hostname, const char *port);
void send_header(int sockfd, const char *host, const char *port, const char *path);
void process_response(int sockfd);

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int sendall(int s, char *buf, int *len) {
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
}

void process_input(const char *input, char *host, char *port, char *path) {
    char info[strlen(input)];

    if (strncmp(input,"http://",7) == 0) {
        strcpy(info,input+7);
    }
    else {
        printf("[ERROR]: Missing 'http://'\n");
        return;
    }
    
    char *p;
    p = strchr(info,'/');
    if (p != NULL) {
        strncpy(host,info,p-info);
        strcpy(path,p);
    }
    else {
        strcpy(path,"/index.html");
    }

    p = strchr(host,':');
    if (p != NULL) {
        strcpy(port,p+1);
        host[p-host] = '\0';
    }
    else {
        strcpy(port,"80");
    }

	printf("[INFO]: Attempting to download %s from %s:%s\n", path, host, port);
}

int read_socket(int socket, char *buf, int buf_size){
    int recv_len = 0;   // recv char length
    char c = '\0';      // store char read from socket
    int i = 0;          // store buf index & length

    // buf_size - 1 because null terminating buffer
    while ((i < buf_size-1) && (c != '\n')) {
        recv_len = recv(socket,&c,1,0);         // read 1 char into char c
        if (recv_len > 0) {
            if (c == '\r') {                    // if recv char '\r'
                // Reference: https://pubs.opengroup.org/onlinepubs/007904975/functions/recv.html
                recv_len = recv(socket,&c,1,MSG_PEEK);
                if ((recv_len > 0) && (c == '\n')) recv(socket,&c,1,0);
                else c = '\n';                  // add char '\n' manually
            }
            buf[i] = c;                         // store char into buf
            ++i;
        } else {
            c = '\n';
        }
    }
    buf[i] = '\0';

    return i;
}

int connect_server(const char *hostname, const char *port) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;   // structs for getting address info
    int rv;                                 // dummy variable for error checking
    char s[INET6_ADDRSTRLEN];               // buffer for server address string

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // set up the addrinfo struct for the server
    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 0;
    }

    // loop through servinfo results and connect to the first one we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("[HTTP CLIENT]: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("[HTTP CLIENT]: connect");
            continue;
        }

        break;
    }

    // if we reached the end, print an error and quit
    if (p == NULL) {
        fprintf(stderr, "[HTTP CLIENT]: unable to connect to %s\n", hostname);
        return 0;
    }

    // print the connected host address
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr),
            s, sizeof(s));
    printf("[HTTP CLIENT]: connected to %s\n", s);

    // all done with this structure, so we can free it
    freeaddrinfo(servinfo);

    return sockfd;
}

void send_header(int sockfd, const char *host, const char *port, const char *path) {
    char header[1024];
    printf("[HTTP CLIENT]: sending header to server\n");

    sprintf(header, "GET %s HTTP/1.1\r\n", path);
    //send(sockfd, header, strlen(header), 0);
    //sprintf(header, "Host: %s:%s\r\n", host, port);
    //send(sockfd, header, strlen(header), 0);
    //strcpy(header, "Connection: close\r\n");
    //send(sockfd, header, strlen(header), 0);
    //strcpy(header, "\r\n");
    send(sockfd, header, strlen(header), 0);
}

void process_response(int sockfd) {
    printf("[HTTP CLIENT]: reading response...\n");

    char buf[4096];   	// buffer for received data
	char status[32];		// buffer for status code

    int read_obj = 0;

	read_obj = read_socket(sockfd,buf,sizeof buf);
	for (int i = 0; i < read_obj; i++) {
		// copy everything after the first space, excluding the trailing \n
		if (buf[i] == ' ') {
			strcpy(status, buf+i+1);
			break;
		}
	}

	while((read_obj = read_socket(sockfd,buf,sizeof buf)) != 0) {
		if (strcmp(buf,"\n") == 0)
			break;
    }

	// if we got any status other than "200 OK" then something went wrong
	if (strncmp(status, "200 OK", 6) != 0) {
		fprintf(stderr, "[ERROR]: Unable to download the file %s\n",status);
		return;
	}

	FILE *f, *f_out = NULL;

	f = fdopen(sockfd,"rb");
	f_out = fopen("output","wb");
	
    do {
		read_obj = fread(buf,1,sizeof buf,f);
		if ((read_obj < sizeof buf) && !feof(f)) {
		    fprintf(stderr, "An error occured while reading from the socket.\n");
			break;
		}
		fwrite(buf,1,read_obj,f_out);
	} while(!feof(f));

    fclose(f_out);
}

int main(int argc, char* argv[]) {
    int sockfd;             // file descriptor for socket

    char hostname[256];     // char hostname
    char port[6];           // char ports # from 0-65535
    char path[256];         // char file path

    memset(hostname,0,256);
    memset(port,0,6);
    memset(path,0,256);

    if (argc != 2) {
        fprintf(stderr, "[USAGE]: ./http_client http://<host>:<port>/<path>\n");
        exit(1);
    }

    // get the hostname, port number, and file path
    process_input(argv[1], hostname, port, path);

    // create a new socket connection to the server
    if ((sockfd = connect_server(hostname, port)) == 0) {
        exit(2);
    }

    send_header(sockfd,hostname,port,path); // send header to server

    process_response(sockfd);               // process response from server

    close(sockfd);

    return 0;
}
