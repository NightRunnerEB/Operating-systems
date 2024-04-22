#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_VISITORS 25
#define PAINTINGS 5
#define MAX_PER_PAINTING 5
#define TOTAL_VISITORS 100

int semid;
int shmid;
int *visitor_count;

void initialize_resources() {
    key_t key = ftok("shmfile", 65);
    shmid = shmget(key, sizeof(int) * PAINTINGS, 0666|IPC_CREAT);
    visitor_count = (int*) shmat(shmid, (void*)0, 0);

    // Инициализация семафоров System V
    semid = semget(key, 1 + PAINTINGS, 0666|IPC_CREAT);
    semctl(semid, 0, SETVAL, MAX_VISITORS);
    for (int i = 1; i <= PAINTINGS; i++) {
        semctl(semid, i, SETVAL, MAX_PER_PAINTING);
    }
}

void cleanup_resources() {
    semctl(semid, 0, IPC_RMID, NULL);
    shmdt(visitor_count);
    shmctl(shmid, IPC_RMID, NULL);
}

void visitor(int id) {
    struct sembuf buf;

    buf.sem_num = 0;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    semop(semid, &buf, 1);

    for (int i = 0; i < PAINTINGS; i++) {
        buf.sem_num = i + 1;
        semop(semid, &buf, 1);

        sleep(1 + rand() % 3);

        buf.sem_op = 1;
        semop(semid, &buf, 1);
    }   

    buf.sem_num = 0;
    buf.sem_op = 1;
    semop(semid, &buf, 1);
}

int main() {
    initialize_resources();
    srand(time(NULL));

    for (int i = 0; i < TOTAL_VISITORS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            visitor(i);
            exit(0);
        } else if (pid < 0) {
            perror("Failed to fork");
            break;
        }
        sleep(rand() % 2);
    }

    while (wait(NULL) > 0);
    cleanup_resources();
    return 0;
}
