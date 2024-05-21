#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MONITOR_PORT 9090

void send_to_monitor(int monitor_socket, const char *message) {
    send(monitor_socket, message, strlen(message), 0);
}

int main(int argc, char const *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <monitor_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *monitor_ip = argv[3];

    int monitor_socket = 0;
    struct sockaddr_in monitor_addr;
    
    if ((monitor_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    monitor_addr.sin_family = AF_INET;
    monitor_addr.sin_port = htons(MONITOR_PORT);

    if (inet_pton(AF_INET, monitor_ip, &monitor_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(monitor_socket, (struct sockaddr *)&monitor_addr, sizeof(monitor_addr)) < 0) {
        perror("Connection to monitor failed");
        exit(EXIT_FAILURE);
    }

    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "Client connected to server %s on port %d\n", server_ip, port);
    send_to_monitor(monitor_socket, buffer);

    while (read(sock, buffer, 1024) > 0) {
        send_to_monitor(monitor_socket, buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);
    close(monitor_socket);
    return 0;
}
