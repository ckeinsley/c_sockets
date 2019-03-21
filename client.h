
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <arpa/inet.h>
#define MAXDATASIZE 150

int connect_to_server(char* hostname, char* port);
void communicate_with_server(int sockfd);
char* recv_command();
int handle_command(int fd, char* command);
void send_command(int fd, char* text);