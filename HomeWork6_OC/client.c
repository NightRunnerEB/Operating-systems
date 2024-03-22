#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SHM_KEY 0x1234 // Ключ разделяемой памяти
#define MEMORY_SIZE 1024 // Размер сегмента разделяемой памяти

int main() {
    int shmid;
    int *data;
    int number;
    
    // Инициализация генератора случайных чисел
    srand(time(NULL));
    
    // Подключаемся к сегменту разделяемой памяти
    shmid = shmget(SHM_KEY, MEMORY_SIZE, 0644);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    
    data = (int *)shmat(shmid, NULL, 0);
    if (data == (int *)(-1)) {
        perror("shmat");
        exit(1);
    }
    
    // Генерируем и отправляем случайные числа
    for (int i = 0; i < 10; ++i) { // Пример: отправляем 10 случайных чисел
        while (*data != 0) {
            sleep(1); // Ожидаем, пока сервер не прочитает предыдущее число
        }
        number = rand() % 1000; // Генерируем случайное число от 0 до 999
        *data = number;
        printf("Отправлено число: %d\n", number);
        sleep(1);
    }
    
    // Сигнал серверу о завершении
    *data = -1;
    
    // Отсоединяемся от разделяемой памяти
    if (shmdt(data) == -1) {
        perror("shmdt");
        exit(1);
    }
}
