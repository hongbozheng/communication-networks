#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

// macro
#define BACKLOG                 10  // how many pending connections queue will hold
#define REQUEST_ARG_NUM         3
#define URI_FIRST_CHAR_INDEX    0

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

#endif
