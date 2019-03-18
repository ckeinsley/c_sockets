
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
#define MAXDATASIZE 100

int connect_to_server(char* hostname, char* port);
void communicate_with_server(int sockfd);
char* recv_command();
int handle_command(char* command);
int startswith(char* pre, char* test);