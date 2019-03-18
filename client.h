
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
#define MAXDATASIZE 150

int connect_to_server(char* hostname, char* port);
void communicate_with_server(int sockfd);
char* recv_command();
int handle_command(int fd, char* command);
void send_command(int fd, char* text);