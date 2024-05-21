#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MONITOR_PORT 9090

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <monitor_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *monitor_ip = argv[1];

    int sock = 0;
    struct sockaddr_in monitor_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    monitor_addr.sin_family = AF_INET;
    monitor_addr.sin_port = htons(MONITOR_PORT);

    if (inet_pton(AF_INET, monitor_ip, &monitor_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&monitor_addr, sizeof(monitor_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    char client_type = 'O';
    if (send(sock, &client_type, 1, 0) < 0) {
        perror("send client type failed");
        return -1;
    }

    char buffer[1024];
    while (read(sock, buffer, 1024) > 0) {
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);
    return 0;
}
