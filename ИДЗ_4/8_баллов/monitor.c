#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MONITOR_PORT 9090
#define MAX_CLIENTS 100

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in clients[MAX_CLIENTS];
int num_clients = 0;

void broadcast_message(const char *message, int monitor_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; ++i) {
        if (sendto(monitor_socket, message, strlen(message), 0, (struct sockaddr *)&clients[i], sizeof(clients[i])) < 0) {
            perror("send failed");
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_observer(void *arg) {
    struct sockaddr_in *observer_addr = (struct sockaddr_in *)arg;

    pthread_mutex_lock(&clients_mutex);
    if (num_clients < MAX_CLIENTS) {
        clients[num_clients++] = *observer_addr;
    } else {
        printf("Max clients reached. Connection refused.\n");
        pthread_mutex_unlock(&clients_mutex);
        return NULL;
    }
    pthread_mutex_unlock(&clients_mutex);

    printf("Observer client connected\n");

    return NULL;
}

void *handle_server_connection(void *arg) {
    int monitor_socket = *(int*)arg;
    char buffer[1024];
    int read_size;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    while ((read_size = recvfrom(monitor_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_len)) > 0) {
        buffer[read_size] = '\0';
        printf("%s", buffer);

        if (buffer[0] == 'O') {
            pthread_t thread;
            struct sockaddr_in *observer_addr = malloc(sizeof(struct sockaddr_in));
            *observer_addr = client_addr;
            pthread_create(&thread, NULL, handle_observer, observer_addr);
            pthread_detach(thread);
        } else {
            broadcast_message(buffer, monitor_socket);
        }

        memset(buffer, 0, sizeof(buffer));
    }

    return NULL;
}

int main(int argc, char const *argv[]) {
    int monitor_socket;
    struct sockaddr_in address;
    int opt = 1;

    if ((monitor_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(monitor_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(MONITOR_PORT);

    if (bind(monitor_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Monitor service is listening on port %d\n", MONITOR_PORT);

    pthread_t thread;
    pthread_create(&thread, NULL, handle_server_connection, (void*)&monitor_socket);
    pthread_detach(thread);

    while (1) {
        // Бесконечный цикл для поддержания работы сервиса
        sleep(1);
    }

    close(monitor_socket);
    return 0;
}
