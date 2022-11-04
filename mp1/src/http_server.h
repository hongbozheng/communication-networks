#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

// macro
#define BACKLOG                 10  // how many pending connections queue will hold
#define REQUEST_ARG_NUM         3
#define URI_FIRST_CHAR_INDEX    0

// function signatures
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
int read_socket(int sock, char *buf, int size);
void file_not_found(int client);
void file_found(int client, const char *filename);
void unknown_request(int client);
void send_file(int client, const char *filename);
int bind_server(const char *port);
int process_request(char *request, char *method, char *URI, char *version);
void handle_client(int client);
int main(int argc, char *argv[]);

#endif