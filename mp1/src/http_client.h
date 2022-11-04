#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

// macro
#define MAXDATASIZE 1024 // max # of bytes we can get at once

// function signatures
void *get_in_addr(struct sockaddr *sa);
void process_input(const char *input, char *host, char *port, char *path);
int read_socket(int socket, char *buf, int buf_size);
int connect_server(const char *hostname, const char *port);
void send_header(int sockfd, const char *host, const char *port, const char *path);
void process_response(int sockfd);
int main(int argc, char* argv[]);

#endif