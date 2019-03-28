#include <payload.h>

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

char* recieve_payload(int fd, int payload_size, char* payload) {
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
    return payload;
}

int send_payload_size(int fd, int payload_size) {
    int32_t converted_payload = htonl(size_of_payload);
    int sent = 0;
    int totalToSend = sizeof(converted_payload);
    char *data = (char *)&converted_payload;

    // send size of data
    do {
        sent = send(fd, data, totalToSend, 0);
        if (sent < 0) {
            perror("send int");
        }
        data += sent;
        totalToSend -= sent;
    } while (sent > 0);

    // get size confirmation
    char response[MAXDATASIZE];
    if ((recv(fd, response, MAXDATASIZE - 1, 0)) == -1) {
        perror("Client failed to recieve. Aborting");
        exit(-1);
    }
}

int send_payload(int fd, int payload_size, char* payload) {
    int32_t converted_payload = htonl(payload_size);
    int sent = 0;
    int totalToSend = sizeof(converted_payload);

    // send data
    totalToSend = payload_size;
    sent = 0;
    data = buf;
    do {
        sent = send(fd, data, totalToSend, 0);
        if (sent < 0) {
            perror("send data");
        }
        data += sent;
        totalToSend -= sent;
    } while (sent > 0);
}