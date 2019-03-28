int receive_payload_size(int fd);
char* recieve_payload(int fd, int payload_size, char* payload);
int send_payload_size(int fd, int payload_size);
int send_payload(int fd, int payload_size, char* payload);
