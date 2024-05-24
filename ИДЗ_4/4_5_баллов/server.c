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

typedef struct {
    int id;
    struct sockaddr_in address;
    socklen_t addr_len;
} Visitor;

pthread_mutex_t gallery_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t gallery_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t painting_cond[NUM_PAINTINGS];
int visitors_in_gallery = 0;
int visitors_at_painting[NUM_PAINTINGS] = {0};
int total_clients = 0;
int server_fd;

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
        sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&visitor->address, visitor->addr_len);
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
    sendto(server_fd, buffer, strlen(buffer), 0, (struct sockaddr *)&visitor->address, visitor->addr_len);
    free(visitor);

    return NULL;
}

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int port = atoi(argv[1]);

    struct sockaddr_in address;
    int opt = 1;

    for (int i = 0; i < NUM_PAINTINGS; ++i) {
        pthread_cond_init(&painting_cond[i], NULL);
    }

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
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", port);

    int visitor_id = 0;
    while (total_clients < MAX_CLIENTS) {
        Visitor *visitor = (Visitor *)malloc(sizeof(Visitor));
        visitor->id = visitor_id++;
        visitor->addr_len = sizeof(visitor->address);

        char buffer[1024];
        recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&visitor->address, &visitor->addr_len);

        total_clients++;
        
        pthread_t thread;
        pthread_create(&thread, NULL, handle_visitor, visitor);
        pthread_detach(thread);
    }

    printf("Рабочий день закончен\n");
    close(server_fd);
    return 0;
}
