
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

#include "payload.h"
#define MAXDATASIZE 150
#define HELP_TEXT "::::CrapTP Commands::::\n\
help               'displays this message'\n\
echo <string>      'server sends <string> back to client and client prints\n\
ls                 'lists files in client-files folder on client'\n\
rls                'lists files in server-files folder on server\n\
iWant <filename>   'downloads <filename> from server and saves under client-files\n\
uTake <filename>   'uploads <filename> from client to the server's server-files directory\n\
quit()             'closes the client'\n\
;;;                'same as quit()'\n"

int connect_to_server(char* hostname, char* port);
void communicate_with_server(int sockfd);
char* recv_command();
int handle_command(int fd, char* command);
void send_command(int fd, char* text);
int startswith(char *pre, char *test);
void list_files();
void recv_file(int fd, char* command);
void send_file(int fd, char* command);
void send_to_server(int fd, char* buf, int size, char* command); 
int check_error(char* buf);