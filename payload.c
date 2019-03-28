#include "payload.h"

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

    send_confirmation(fd);

    return ntohl(nonconverted_payload_size);
}

void receive_payload(int fd, int payload_size, char* payload) {
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
    payload[payload_size] = '\0';
}

void send_payload_size(int fd, int payload_size) {
    int32_t converted_payload = htonl(payload_size);
    int sent = 0;
    int totalToSend = sizeof(converted_payload);
    char *data = (char *)&converted_payload;

    // send size of data
    do {
        sent = send(fd, data, totalToSend, 0);
        if (sent < 0) {
            perror("send int");
            exit(-1);
        }
        data += sent;
        totalToSend -= sent;
    } while (sent > 0);

    // get size confirmation
    get_confirmation(fd);
}

void send_payload(int fd, int payload_size, char* payload) {
    int sent = 0;
    char* data = payload;
    do {
        sent = send(fd, data, payload_size, 0);
        if (sent < 0) {
            perror("send data");
            exit(-1);
        }
        data += sent;
        payload_size -= sent;
    } while (sent > 0);
}

void send_confirmation(int fd) {
    if ((send(fd, "Yep", 4, 0)) == -1) {
        perror("couldn't send confirmation");
        exit(-1);
    }
}

void get_confirmation(int fd) {
    char response[MAXDATASIZE];
    if ((recv(fd, response, MAXDATASIZE - 1, 0)) == -1) {
        perror("Client failed to recieve. Aborting");
        exit(-1);
    }
}