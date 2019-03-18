#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define BACKLOG 10

int bind_socket();
int setup(int* sockfd);
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
void run(int sockfd);
void handle_client(int fd);