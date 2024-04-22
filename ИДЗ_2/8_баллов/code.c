    #include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>

#define MAX_VISITORS 25
#define PAINTINGS 5
#define MAX_PER_PAINTING 5
#define TOTAL_VISITORS 100

sem_t *gallery_sem;
sem_t *painting_sem[PAINTINGS];
int *visitor_count;
int shm_fd;

void initialize_resources() {
    // Создаем семафор для галереи
    gallery_sem = sem_open("/gallery_sem", O_CREAT | O_EXCL, 0644, MAX_VISITORS);
    if (gallery_sem == SEM_FAILED) {
        perror("Failed to open semaphore for gallery");
        exit(EXIT_FAILURE);
    }

    // Создаем семафоры для каждой картины
    char sem_name[20];
    for (int i = 0; i < PAINTINGS; i++) {
        sprintf(sem_name, "/painting_sem_%d", i);
        painting_sem[i] = sem_open(sem_name, O_CREAT | O_EXCL, 0644, MAX_PER_PAINTING);
        if (painting_sem[i] == SEM_FAILED) {
            perror("Failed to open semaphore for painting");
            exit(EXIT_FAILURE);
        }
    }

    // Создаем разделяемую память
    shm_fd = shm_open("/gallery_shm", O_CREAT | O_RDWR | O_EXCL, 0644);
    if (shm_fd == -1) {
        perror("Failed to create shared memory segment");
        exit(EXIT_FAILURE);
    }
    ftruncate(shm_fd, sizeof(int) * PAINTINGS);
    visitor_count = mmap(NULL, sizeof(int) * PAINTINGS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (visitor_count == MAP_FAILED) {
        perror("Failed to map shared memory");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < PAINTINGS; i++) {
        visitor_count[i] = 0;
    }
}

void cleanup_resources() {
    // Закрываем семафоры
    sem_close(gallery_sem);
    sem_unlink("/gallery_sem");
    for (int i = 0; i < PAINTINGS; i++) {
        char sem_name[20];
        sprintf(sem_name, "/painting_sem_%d", i);
        sem_close(painting_sem[i]);
        sem_unlink(sem_name);
    }

    // Освобождаем разделяемую память
    munmap(visitor_count, sizeof(int) * PAINTINGS);
    shm_unlink("/gallery_shm");
    close(shm_fd);
}

void handle_sigint(int sig) {
    cleanup_resources();
    printf("Cleaned up resources. Exiting.\n");
    exit(EXIT_SUCCESS);
}

void visitor(int id) {
    printf("Visitor %d trying to enter the gallery.\n", id);
    sem_wait(gallery_sem);  // Вход в галерею

    for (int i = 0; i < PAINTINGS; i++) {
        printf("Visitor %d waiting to view painting %d.\n", id, i);
        sem_wait(&painting_sem[i]);  // Подход к картине

        // Просмотр картины
        printf("Visitor %d is viewing painting %d.\n", id, i);
        sleep(1 + rand() % 3);  // Имитация времени просмотра

        sem_post(&painting_sem[i]);  // Освобождение картины
        printf("Visitor %d finished viewing painting %d.\n", id, i);
    }

    sem_post(gallery_sem);  // Выход из галереи
    printf("Visitor %d leaving the gallery.\n", id);
    exit(0);
}

int main() {
    initialize_resources();
    signal(SIGINT, handle_sigint);
    srand(time(NULL));

    for (int i = 0; i < TOTAL_VISITORS; i++) {
        pid_t pid = fork();
        if (pid == 0) {  // Дочерний процесс
            visitor(i);
        } else if (pid < 0) {
            perror("Failed to fork");
            break;
        }
        // Ограничение на частоту создания посетителей
        sleep(rand() % 2);
    }

    while (wait(NULL) > 0);  // Ожидание завершения всех дочерних процессов
    cleanup_resources();
    return 0;
}
