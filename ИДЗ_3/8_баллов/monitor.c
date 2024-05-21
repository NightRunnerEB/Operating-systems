#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MONITOR_PORT 9090
#define MAX_CLIENTS 100

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int clients[MAX_CLIENTS];
int num_clients = 0;

void broadcast_message(const char *message) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; ++i) {
        if (send(clients[i], message, strlen(message), 0) < 0) {
            perror("send failed");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_server_connection(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char buffer[1024];
    int read_size;

    while ((read_size = recv(sock, buffer, 1024, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("%s", buffer);
        broadcast_message(buffer);
        memset(buffer, 0, sizeof(buffer));
    }

    close(sock);
    free(socket_desc);
    return NULL;
}

void *handle_observer(void *socket_desc) {
    int sock = *(int*)socket_desc;

    pthread_mutex_lock(&clients_mutex);
    if (num_clients < MAX_CLIENTS) {
        clients[num_clients++] = sock;
    } else {
        printf("Max clients reached. Connection refused.\n");
        close(sock);
        free(socket_desc);
        pthread_mutex_unlock(&clients_mutex);
        return NULL;
    }
    pthread_mutex_unlock(&clients_mutex);

    printf("Observer client connected: %d\n", sock);

    char buffer[1024];
    while ((recv(sock, buffer, 1024, 0)) > 0) {
        // Обрабатываем входящие данные, если необходимо
    }

    printf("Observer client disconnected: %d\n", sock);

    close(sock);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; ++i) {
        if (clients[i] == sock) {
            clients[i] = clients[--num_clients];
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    free(socket_desc);
    return NULL;
}

void *handle_connection(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char client_type;
    if (recv(sock, &client_type, 1, 0) <= 0) {
        perror("recv failed");
        close(sock);
        free(socket_desc);
        return NULL;
    }

    if (client_type == 'S') {
        handle_server_connection(socket_desc);
    } else if (client_type == 'O') {
        handle_observer(socket_desc);
    } else {
        printf("Unknown client type: %c\n", client_type);
        close(sock);
        free(socket_desc);
    }

    return NULL;
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
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

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Monitor service is listening on port %d\n", MONITOR_PORT);

    while (1) {
        int *new_sock = (int*)malloc(sizeof(int));
        *new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (*new_sock < 0) {
            perror("accept");
            free(new_sock);
            exit(EXIT_FAILURE);
        }

        printf("New connection accepted: %d\n", *new_sock);

        pthread_t thread;
        pthread_create(&thread, NULL, handle_connection, (void*)new_sock);
        pthread_detach(thread);
    }

    close(server_fd);
    return 0;
}
