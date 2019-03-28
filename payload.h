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
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#define MAXDATASIZE 150

int receive_payload_size(int fd);
void receive_payload(int fd, int payload_size, char* payload);
void send_payload_size(int fd, int payload_size);
void send_payload(int fd, int payload_size, char* payload);
void send_confirmation(int fd);
void get_confirmation(int fd);
