#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);

    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    printf("Connected to gallery\n");

    char buffer[1024] = {0};
    socklen_t addr_len = sizeof(serv_addr);

    sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, addr_len);

    while (recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &addr_len) > 0) {
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);
    return 0;
}
