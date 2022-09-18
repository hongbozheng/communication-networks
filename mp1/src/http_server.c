/**
 * FILENAME: http_server.c
 * 
 * DESCRIPTION: a http server application
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define BACKLOG 10	 		// how many pending connections queue will hold
#define REQUEST_ARG_NUM 3
#define URI_FIRST_CHAR_INDEX 0

// function signatures
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
int sock_getline(int sock, char *buf, int size);
int main(int argc, char *argv[]);
int bind_server(const char *port);
void handle_client(int client);
int process_request(char *request, char *method, char *URI, char *version);
void respond_bad_request(int client);
void respond_not_found(int client);
void respond_ok(int client, const char *filename);
void send_file(int client, const char *filename);


void sigchld_handler(int s) {
	(void)s;
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
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

void file_not_found(int client) {
	char buf[1024];

	sprintf(buf, "HTTP/1.1 404 Not Found\r\n");
	send(client,buf,strlen(buf),0);
	sprintf(buf,"Content-Type: text/html\r\n");
	send(client,buf,strlen(buf),0);
	sprintf(buf,"The server could not complete the request \
            because the request file is unavailable or nonexsistent.\r\n");
	send(client,buf,strlen(buf),0);
	sprintf(buf,"\r\n");
	send(client, buf, strlen(buf),0);
}

void file_found(int client, const char *filename) {
	char buf[1024];

	sprintf(buf,"HTTP/1.1 200 OK\r\n");
	send(client,buf,strlen(buf),0);
	sprintf(buf,"Content-Type: text\r\n");
	send(client,buf,strlen(buf),0);
    sprintf(buf,"Found file %s\r\n",filename);
	send(client,buf,strlen(buf),0);
	sprintf(buf,"\r\n");
	send(client,buf,strlen(buf),0);
}

void unknown_request(int client) {
	char buf[1024];

	sprintf(buf,"HTTP/1.1 400 Bad Request\r\n");
	send(client,buf,strlen(buf),0);
	sprintf(buf,"Content-Type: text/html\r\n");
	send(client,buf,strlen(buf),0);
    sprintf(buf,"The server could not complete the request \
            because the request is unknown.\r\n");
	send(client, buf, strlen(buf), 0);
	sprintf(buf,"\r\n");
	send(client, buf, strlen(buf), 0);
}

void send_file(int client, const char *filename) {
	FILE *f, *f_out = NULL;     // create ptr for read file & output file

	f= fopen(filename,"rb");    // try open read file
	if (f == NULL) {            // read file does not exist
		file_not_found(client);
		return;
	}
	file_found(client,filename);   // why is this not printing anything ??? make sense

	f_out = fdopen(client,"wb");
	char buf[4096];
	int read_obj = 0;
	do {
		read_obj = fread(buf,1,sizeof buf,f);
		if ((read_obj < sizeof buf) && !feof(f)) {
				fprintf(stderr, "[ERROR]: An error occured while reading from file %s\n",filename);
				break;
	    }
		fwrite(buf,1,read_obj,f_out);
	} while(!feof(f));

	fclose(f_out);  // close output file
	fclose(f);      // close read file
}

int bind_server(const char *port) {
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int yes = 1;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;        // use my IP

	if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("[HTTP_SERVER]: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("[HTTP_SERVER]: bind");
			continue;
		}

		break;
	}

	if (p == NULL)  {
		fprintf(stderr, "[HTTP_SERVER]: failed to bind\n");
		return 2;
	}

	printf("[HTTP SERVER]: socket bound to port %s\n", port);

	freeaddrinfo(servinfo); // all done with this structure

	return sockfd;
}

int process_request(char *request, char *method, char *URI, char *version) {
    char *pch;
    int argc = 0;
    
    pch = strtok(request," ");
    while (pch != NULL){            // start to parse http request args
        switch (argc) {             // Reference: https://cplusplus.com/reference/cstring/strtok/
            case 0:
                strcpy(method,pch);
                break;
            case 1:
                strcpy(URI,pch);
                break;
            case 2:
                strcpy(version,pch);
                break;
            default:
                printf("[ERROR]: Too many args for REQUEST! Only 3 is needed !\n");
                return -1;
        }
        printf("%s\n",pch);
        ++argc;
        pch = strtok(NULL," ");
    }
	return 0;
}

void handle_client(int client) {
	char http_request[1024];    // read income http request
	int read_obj;
	
    read_obj = read_socket(client,http_request,sizeof http_request);
	printf("[REQUEST]: %s\n",http_request);
    
    char discard[256];
	discard[0] = 'A'; discard[1] = '\0';
	while((read_obj > 0) && strcmp(discard, "\n")) {
        read_socket(client, discard, sizeof(discard));
    }
	
    // parse request line from header
	char method[32];
	char uri[256];
	char version[32];

	int request_check = process_request(http_request, method, uri, version);
	if (request_check == -1) {
		unknown_request(client);
		return;
	}

	if (strcmp(method,"GET") != 0) {
		unknown_request(client);
		return;
	}

	printf("Method: %s\n", method);
	printf("URI: %s\n", uri);
	printf("Version: %s\n", version);
    
    // remove the beginning char '/'
    // Reference: https://stackoverflow.com/questions/5457608/how-to-remove\
                  -the-character-at-a-given-index-from-a-string-in-c
    memmove(&uri[URI_FIRST_CHAR_INDEX],&uri[URI_FIRST_CHAR_INDEX+1],
            strlen(uri)-URI_FIRST_CHAR_INDEX);

	send_file(client,uri);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
        fprintf(stderr, "[USAGE]: ./http_server <port>\n");
        exit(1);
    }

    int sockfd, new_fd;                 // listen on sock_fd, new connection on new_fd
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	char s[INET6_ADDRSTRLEN];

	sockfd = bind_server(argv[1]);      // bind_server to port #

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler;    // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("[SERVER]: waiting for connections...\n\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("[SERVER]: got connection from %s\n", s);

		if (!fork()) {  	        // this is the child process
			close(sockfd); 	        // child doesn't need the listener
			handle_client(new_fd);
			close(new_fd);
			exit(0);
		}
		close(new_fd);              // parent doesn't need this
	}

	return 0;
}
