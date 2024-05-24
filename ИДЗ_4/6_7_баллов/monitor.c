#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MONITOR_PORT 9090

void *handle_connection(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[1024];
    int read_size;
    while ((read_size = recv(sock, buffer, 1024, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("%s", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    close(sock);
    free(socket_desc);
    return NULL;
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(MONITOR_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Monitor service is listening on port %d\n", MONITOR_PORT);

    while (1) {
        char buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        int read_size = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (read_size > 0) {
            buffer[read_size] = '\0';
            printf("%s", buffer);
        }
    }

    close(server_fd);
    return 0;
}
