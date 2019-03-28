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
    if (strcmp(command, "quit()") == 0 || strcmp(command, ";;;") == 0) {
        printf("Closing Connection\n");
        close(fd);
        return 1;
    }

    if (startswith("iWant", command)) {
        recv_file(fd, command);
        return 0;
    }

    if (startswith("uTake", command)) {
        send_file(fd, command);
        return 0;
    }

    if (command[0] != '\0') {
        send_command(fd, command);
    }
    return 0;
}

void send_command(int fd, char* text) {
    // Send command
    if (send(fd, text, strlen(text), 0) == -1) {
        perror("send");
        exit(-1);
    }

    // Receive payload size
    int payload_size = receive_payload_size(fd);

    // Receive data
    char payload[payload_size + 1];
    receive_payload(fd, payload_size, payload);
    printf("%s\n", payload);
}

void recv_file(int fd, char* command) {
    // send download request
    if (send(fd, command, strlen(command), 0) == -1) {
        perror("send");
        exit(-1);
    }

    // receive size
    int payload_size = receive_payload_size(fd);
    printf("size of file should be %d\n", payload_size);

    // receive data
    char payload[payload_size + 1];
    receive_payload(fd, payload_size, payload);

    // create file
    char buf[MAXDATASIZE];
    sprintf(buf, "client-files/%s", command+6);
    FILE* fp = fopen(buf, "wb");
    if (!fp) {
        printf("Unable to open file to write\n");
        return;
    }

    for (int i = 0; i < payload_size; i++) {
        fputc(payload[i], fp);
    }
    fclose(fp);

    printf("File downloaded\n");
}

void send_file(int fd, char* command) {
    char buf[MAXDATASIZE];
    sprintf(buf,"client-files/%s", command+6);

    FILE *f = fopen(buf, "rb");
    if (!f) {
        printf("Unable to open file to read\n");
        return;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    printf("read file of size %li\n", fsize);
    send_to_server(fd, string, fsize, command);
}

void send_to_server(int fd, char* buf, int size, char* command) {
    // send upload request
    if (send(fd, command, strlen(command), 0) == -1) {
        perror("send");
        exit(-1);
    }

    // get size confirmation
    get_confirmation(fd);
    
    // send size of data
    send_payload_size(fd, size);

    // send data
    send_payload(fd, size, buf);

    printf("File uploaded\n");
}

int startswith(char* pre, char* test) {
    return strncmp(pre, test, strlen(pre)) == 0;
}
