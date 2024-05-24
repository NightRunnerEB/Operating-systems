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
    struct sockaddr_in monitor_addr, local_addr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    monitor_addr.sin_family = AF_INET;
    monitor_addr.sin_port = htons(MONITOR_PORT);

    if (inet_pton(AF_INET, monitor_ip, &monitor_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Привязываем сокет к локальному адресу и порту
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(0); // Автоматический выбор порта

    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("Bind failed");
        return -1;
    }

    char client_type = 'O';
    if (sendto(sock, &client_type, 1, 0, (struct sockaddr *)&monitor_addr, sizeof(monitor_addr)) < 0) {
        perror("send client type failed");
        return -1;
    }

    char buffer[1024];
    while (1) {
        socklen_t addr_len = sizeof(monitor_addr);
        int len = recvfrom(sock, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&monitor_addr, &addr_len);
        if (len > 0) {
            buffer[len] = '\0';
            printf("%s", buffer);
        } else {
            perror("recvfrom failed");
            break;
        }
    }

    close(sock);
    return 0;
}
