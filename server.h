#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#define BACKLOG 10
#define MAXDATASIZE 150

int bind_socket();
int setup(int* sockfd);
void sigchld_handler(int s);
void* get_in_addr(struct sockaddr* sa);
void run(int sockfd);
void handle_client(int fd);
void recv_command(int fd, char* command);
int handle_command(int fd, char* command);
int startswith(char* pre, char* test);
void moby_dick(int fd);
int sendall(int s, char* buf, int* len);
void send_file(int fd, char* file_name);
void list_files(int fd);
void send_to_client(int fd, char* buf, int size);