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

    // Recieve payload size
    int payload_size = receive_payload_size(fd);

    // Send confirmation
    if ((send(fd, "Yep", 4, 0)) == -1) {
        perror("couldn't send confirmation");
        exit(-1);
    }

    // recieve data
    char payload[payload_size + 1];
    recieve_payload(fd, payload_size, payload);
    printf("%s\n", payload);
}

int receive_payload_size(int fd) {
    int32_t nonconverted_payload_size;
    char* data = (char*)&nonconverted_payload_size;
    int left = sizeof(nonconverted_payload_size);
    int received;
    do {
        received = recv(fd, data, left, 0);
        if (received < 0) {
            perror("unable to recieve int");
            exit(-1);
        }
        left -= received;
        data += received;

    } while (left > 0);
    return ntohl(nonconverted_payload_size);
}

void recieve_payload(int fd, int payload_size, char* payload) {
    int left = payload_size;
    char* data = payload;
    int received = 0;
    do {
        received = recv(fd, data, left, 0);
        if (received < 0) {
            perror("unable to recieve int");
            exit(-1);
        }
        left -= received;
        data += received;

    } while (left > 0);
    payload[payload_size + 1] = '\0';
}

void recv_file(int fd, char* command) {
    // send download request
    if (send(fd, command, strlen(command), 0) == -1) {
        perror("send");
        exit(-1);
    }

    // Recieve payload size
    int payload_size = receive_payload_size(fd);

    printf("size of file should be %d\n", payload_size);

    // Send confirmation
    if ((send(fd, "Yep", 4, 0)) == -1) {
        perror("couldn't send confirmation");
        exit(-1);
    }

    // recieve data
    char payload[payload_size + 1];
    recieve_payload(fd, payload_size, payload);

    FILE* fp = fopen(command+6, "wb");
    if (!fp) {
        printf("Unable to open file to write\n");
        return;
    }

    //fwrite((void*)buf, file_buf_index+1, sizeof(char), fp);
    for (int i = 0; i < payload_size; i++) {
        fputc(payload[i], fp);
    }
    fclose(fp);

    printf("File downloaded\n");
}

void send_file(int fd, char* command) {
    char buf[MAXDATASIZE];
    sprintf(buf,"files/%s", command+6);

    FILE *f = fopen(buf, "rb");
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
    int32_t converted_payload = htonl(size);
    int sent = 0;
    int totalToSend = sizeof(converted_payload);
    char *data = (char *)&converted_payload;

    // send upload request
    if (send(fd, command, strlen(command), 0) == -1) {
        perror("send");
        exit(-1);
    }

    // get size confirmation
    char response[MAXDATASIZE];
    if ((recv(fd, response, MAXDATASIZE - 1, 0)) == -1) {
        perror("Client failed to recieve. Aborting");
        exit(-1);
    }

    // send size of data
    do {
        sent = send(fd, data, totalToSend, 0);
        if (sent < 0) {
            perror("send int");
        }
        data += sent;
        totalToSend -= sent;
    } while (sent > 0);

}

int startswith(char* pre, char* test) {
    return strncmp(pre, test, strlen(pre)) == 0;
}
