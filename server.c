#include "server.h"

char *PORT;
char s[INET6_ADDRSTRLEN];  // Globally track the address of the client connecting so our fork knows

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <PORT>\n", argv[0]);
        exit(1);
    }

    PORT = argv[1];

    int sockfd;
    setup(&sockfd);
    run(sockfd);
    return 0;
}

int setup(int *sockfd) {
    *sockfd = bind_socket();

    if (listen(*sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    struct sigaction sa;
    sa.sa_handler = sigchld_handler;  // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
}

int bind_socket() {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    socklen_t sin_size;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // Loop over linked list returned from getaddrinfo
    // until we can successfully bind a socket
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        // Reusable port
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        // Claim the port
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);  // all done with this
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    return sockfd;
}

void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void run(int sockfd) {
    int new_fd;
    socklen_t sin_size;
    struct sockaddr_storage their_addr;

    printf("server: waiting for connections...\n");

    while (1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) {      // this is the child process
            close(sockfd);  // child doesn't need the listener
            handle_client(new_fd);
        }
        close(new_fd);  // parent doesn't need this
    }
}

void handle_client(int fd) {
    char command[MAXDATASIZE];

    if ((send(fd, "Hello, friend! Welcome to a CrapTP. It's like FTP, but worse!", 62, 0)) == -1) {
        perror("handshake");
    }

    int quit;
    while (1) {
        for (int i = 0; i < MAXDATASIZE; i++) {
            command[i] = 0;
        }
        recv_command(fd, command);
        quit = handle_command(fd, command);
        if (quit) {
            break;
        }
    }

    close(fd);
    exit(0);
}

void recv_command(int fd, char *command) {
    int numbytes;
    if ((numbytes = recv(fd, command, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        exit(1);
    }

    command[numbytes] = '\0';
}

int handle_command(int fd, char *command) {
    char buffer[MAXDATASIZE];
    if (command[0] == '\0') {
        printf("Client at %s Disconnected\n", s);
        return 1;
    }
    printf("Received Command: %s\n", command);

    if (startswith("echo ", command)) {
        send_to_client(fd, command + 5, strlen(command + 5));
        return 0;
    }

    if (strcmp("moby dick", command) == 0) {
        moby_dick(fd);
        return 0;
    }

    if (strcmp("ls", command) == 0) {
        list_files(fd);
        return 0;
    }

    if (startswith("iWant", command)) {
        send_file(fd, command + 6);
        return 0;
    }

    sprintf(buffer, "Unknown Command: %s", command);
    send_to_client(fd, buffer, strlen(buffer));
    return 0;
}

int startswith(char *pre, char *test) {
    return strncmp(pre, test, strlen(pre)) == 0;
}

void moby_dick(int fd) {
    send_file(fd, "moby_dick.txt");
}

void list_files(int fd) {
    char *directory = "./files";
    char buf[1000];
    int buf_index = 0;
    int str_index = 0;

    DIR *d;
    struct dirent *dir;
    d = opendir(directory);
    if (d != NULL) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_name[0] != '.') {
                while (dir->d_name[str_index] != '\0') {
                    buf[buf_index++] = dir->d_name[str_index++];
                }
                str_index = 0;
                buf[buf_index++] = '\n';
            }
        }
        buf[buf_index++] = '\0';
        send_to_client(fd, buf, strlen(buf));
        closedir(d);
    }
}

void send_file(int fd, char *file_name) {
    char buf[MAXDATASIZE];
    sprintf(buf,"files/%s", file_name);

    FILE *f = fopen(buf, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;

    printf("read file of size %li\n", fsize);
    send_to_client(fd, string, fsize);
}

void send_to_client(int fd, char *buf, int size) {
    send_payload_size(fd, size);
    send_payload(fd, size, buf);
}
