#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define MAX_VISITORS 25
#define MAX_PER_PAINTING 5
#define NUM_PAINTINGS 5
#define MAX_CLIENTS 100
#define MONITOR_PORT 9090

typedef struct {
    int id;
    struct sockaddr_in address;
    socklen_t addr_len;
    int monitor_socket;
    struct sockaddr_in monitor_addr;
} Visitor;

pthread_mutex_t gallery_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gallery_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t painting_cond[NUM_PAINTINGS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in clients[MAX_CLIENTS];
int visitors_in_gallery = 0;
int visitors_at_painting[NUM_PAINTINGS] = {0};
int total_clients = 0;
int num_clients = 0;
int server_fd;

void send_to_monitor(int monitor_socket, const char *message, struct sockaddr_in *monitor_addr, socklen_t addr_len) {
    if (sendto(monitor_socket, message, strlen(message), 0, (struct sockaddr *)monitor_addr, addr_len) < 0) {
        perror("send to monitor failed");
    }
}

void shuffle(int *array, size_t n) {
    if (n > 1) {
        size_t i;
        for (i = 0; i < n - 1; i++) {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[i];
            array[i] = array[j];
            array[j] = t;
        }
    }
}

void *handle_visitor(void *arg) {
    Visitor *visitor = (Visitor *)arg;
    srand(time(NULL) ^ (visitor->id << 16)); // Unique seed for each visitor

    // Enter the gallery
    pthread_mutex_lock(&gallery_mutex);
    while (visitors_in_gallery >= MAX_VISITORS) {
        pthread_cond_wait(&gallery_cond, &gallery_mutex);
    }
    visitors_in_gallery++;
    pthread_mutex_unlock(&gallery_mutex);

    char buffer[1024];
    snprintf(buffer, sizeof(buffer), "Visitor %d entered the gallery\n", visitor->id);
    send_to_monitor(visitor->monitor_socket, buffer, &visitor->monitor_addr, visitor->addr_len);

    int paintings[NUM_PAINTINGS] = {0, 1, 2, 3, 4};
    shuffle(paintings, NUM_PAINTINGS);

    for (int i = 0; i < NUM_PAINTINGS; ++i) {
        int painting = paintings[i];

        pthread_mutex_lock(&gallery_mutex);
        while (visitors_at_painting[painting] >= MAX_PER_PAINTING) {
            pthread_cond_wait(&painting_cond[painting], &gallery_mutex);
        }
        visitors_at_painting[painting]++;
        pthread_mutex_unlock(&gallery_mutex);

        snprintf(buffer, sizeof(buffer), "Visitor %d is looking at painting %d\n", visitor->id, painting);
        send_to_monitor(visitor->monitor_socket, buffer, &visitor->monitor_addr, visitor->addr_len);
        sleep(rand() % 3 + 1); // Random time at painting

        pthread_mutex_lock(&gallery_mutex);
        visitors_at_painting[painting]--;
        pthread_cond_broadcast(&painting_cond[painting]);
        pthread_mutex_unlock(&gallery_mutex);
    }

    // Leave the gallery
    pthread_mutex_lock(&gallery_mutex);
    visitors_in_gallery--;
    pthread_cond_broadcast(&gallery_cond);
    pthread_mutex_unlock(&gallery_mutex);

    snprintf(buffer, sizeof(buffer), "Visitor %d is leaving the gallery\n", visitor->id);
    send_to_monitor(visitor->monitor_socket, buffer, &visitor->monitor_addr, visitor->addr_len);
    free(visitor);

    return NULL;
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

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <monitor_ip>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);
    const char *monitor_ip = argv[2];

    int monitor_socket = 0;
    struct sockaddr_in monitor_addr;

    if ((monitor_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    monitor_addr.sin_family = AF_INET;
    monitor_addr.sin_port = htons(MONITOR_PORT);

    if (inet_pton(AF_INET, monitor_ip, &monitor_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address, client_addr;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", port);

    int visitor_id = 0;
    char buffer[1024];
    socklen_t client_addr_len = sizeof(client_addr);

    while (total_clients < MAX_CLIENTS) {
        int len = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (len > 0) {
            buffer[len] = '\0';
            if (buffer[0] == 'O') {
                pthread_t thread;
                struct sockaddr_in *observer_addr = malloc(sizeof(struct sockaddr_in));
                *observer_addr = client_addr;
                pthread_create(&thread, NULL, handle_observer, observer_addr);
                pthread_detach(thread);
            } else {
                Visitor *visitor = (Visitor *)malloc(sizeof(Visitor));
                visitor->id = visitor_id++;
                visitor->address = client_addr;
                visitor->addr_len = client_addr_len;
                visitor->monitor_socket = monitor_socket;
                visitor->monitor_addr = monitor_addr;

                pthread_t thread;
                pthread_create(&thread, NULL, handle_visitor, visitor);
                pthread_detach(thread);

                total_clients++;
            }
        }
    }

    snprintf(buffer, sizeof(buffer), "Рабочий день закончен\n");
    send_to_monitor(monitor_socket, buffer, &monitor_addr, sizeof(monitor_addr));
    close(server_fd);
    close(monitor_socket);
    return 0;
}
