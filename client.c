#include "client.h"

// get sockaddr, IPv4 or IPv6:
void* get_in_addr(struct sockaddr* sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: ./client <hostname> <port>\n");
        exit(1);
    }

    int sockfd = connect_to_server(argv[1], argv[2]);
    communicate_with_server(sockfd);

    return 0;
}

int connect_to_server(char* hostname, char* port) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo);  // all done with this structure

    return sockfd;
}

void communicate_with_server(int sockfd) {
    int numbytes;
    char buf[MAXDATASIZE];
    char* command;

    // Handshake
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';
    printf("%s\n", buf);

    // Handle Commands
    int quit;
    while (1) {
        command = recv_command();
        quit = handle_command(sockfd, command);
        free(command);
        if (quit) {
            break;
        }
    }

    close(sockfd);
}

char* recv_command() {
    char* line;
    size_t n = 0;
    ssize_t read = getline(&line, &n, stdin);

    line[read - 1] = '\0';
    return line;
}

int handle_command(int fd, char* command) {
    if (command[0] != '\0') {
        send_command(fd, command);
    }
    return 0;
}

void send_command(int fd, char* text) {
    int numbytes;
    char buf[MAXDATASIZE];

    if (send(fd, text, strlen(text), 0) == -1)
        perror("send");

    while (1) {
        numbytes = recv(fd, buf, MAXDATASIZE - 1, 0);

        if (numbytes == -1) {
            perror("recv");
            exit(1);
        }
        buf[numbytes] = '\0';
        if (buf[0] == '\0') {
            perror("Server Closed Connection");
            exit(1);
        }

        if (strstr(buf, "~!~DONE~!~") != NULL) { // in case DONE gets stuck on a message
            buf[numbytes - 11] = '\0';
            if (buf[0] == '\0') {
                break;
            }

            printf("Received from server: %s\n", buf);
            break;
        }

        printf("Received from server: %s\n", buf);
    }
}