#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define SHM_KEY 0x1234 // Ключ разделяемой памяти
#define MEMORY_SIZE 1024 // Размер сегмента разделяемой памяти

int main() {
    int shmid;
    int *data;

    // Создаем сегмент разделяемой памяти
    shmid = shmget(SHM_KEY, MEMORY_SIZE, 0644 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }

    // Присоединяем сегмент разделяемой памяти к адресному пространству процесса
    data = (int *)shmat(shmid, NULL, 0);
    if (data == (int *)(-1)) {
        perror("shmat");
        exit(1);
    }

    // Ожидаем данные от клиента
    while (*data != -1) {
        if (*data != 0) { // Проверяем, есть ли новые данные
            printf("Получено число: %d\n", *data);
            *data = 0; // Сбрасываем флаг данных
        }
        sleep(1); // Чтобы не нагружать процессор
    }

    // Отсоединяемся от разделяемой памяти
    if (shmdt(data) == -1) {
        perror("shmdt");
        exit(1);
    }

    // Удаляем сегмент разделяемой памяти
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }

    printf("Сервер завершил работу.\n");
    return 0;
}
