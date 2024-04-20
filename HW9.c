#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int semid;
char pathname[]="HW9.c";
int fd[2];

void child(int* pd, int semid) {
    char result[15] = "\0";
    struct sembuf child_buf = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};

    while (1) {
        if(semop(semid, &child_buf, 1) < 0){
            printf("Can\'t sub 1 from semaphor\n");
            exit(-1);
        }
        read(pd[0], result, 15);
        printf("CHILD: %s\n", result);
        sleep(1);  // Пауза перед ответом

        struct sembuf reply_buf = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};
        if(semop(semid, &reply_buf, 1) < 0){
            printf("Can\'t add 1 to semaphor\n");
            exit(-1);
        }
        write(pd[1], "Child to parent", 15);
        sleep(1);  // Пауза после отправки сообщения
    }
}

void parent(int* pd, int semid) {
    char result[15] = "\0";
    struct sembuf parent_buf = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};

    while (1) {
        write(pd[1], "Parent to child", 15);
        sleep(1);  // Пауза перед ожиданием ответа

        if(semop(semid, &parent_buf, 1) < 0){
            printf("Can\'t sub 1 from semaphor\n");
            exit(-1);
        }
        read(pd[0], result, 15);
        printf("PARENT: %s\n", result);
        sleep(1);  // Пауза после получения сообщения
    }
}

void finish_communication_child(int signal) {
    printf("Child exited\n");
    exit(0);
}

void finish_communication_parent(int signal) {
    close(fd[0]);
    close(fd[1]);
    printf("Pipe deleted\n");

    if (semctl(semid, 0, IPC_RMID, 0) < 0) {
        printf("Can\'t delete semaphore\n");
        exit(-1);
    }
    printf("Sem deleted\n");
    printf("Parent exited\n");
    exit(0);
}

int main() {
    if(pipe(fd) < 0) {
        printf("Pipe error\n");
        return -1;
    }

    key_t key = ftok(pathname, 0);
    if((semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL)) < 0){
        printf("Can\'t create semaphore\n");
        return -1;
    }
    semctl(semid, 0, SETVAL, 0);

    pid_t pid = fork();
    if (pid < 0) {
        printf("Fork error\n");
        return -1;
    } else if (pid == 0) {
        signal(SIGINT, finish_communication_child);
        child(fd, semid);
    } else {
        signal(SIGINT, finish_communication_parent);
        parent(fd, semid);
    }
    return 0;
}
