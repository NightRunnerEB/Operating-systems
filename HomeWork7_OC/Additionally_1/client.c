// client.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

int main () {
    int shm_id;
    char gen_object[] = "gen-memory"; //  имя объекта
    int number;
    int iterations = 11;
    
    if ( (shm_id = shm_open(gen_object, O_CREAT|O_RDWR, 0666)) == -1 ) {
      perror("shm_open");
      return 1;
    } else {
      printf("Object is open: name = %s, id = 0x%x\n", gen_object, shm_id);
    }
    
    srand(time(NULL));
    //получить доступ к памяти
    int* data = mmap(0, sizeof(number), PROT_WRITE|PROT_READ, MAP_SHARED, shm_id, 0);
    if (data == (int*)-1 ) {
      printf("Error getting pointer to shared memory\n");
      return 1;
    }
    
    // Генерируем и отправляем случайные числа
    for (int i = 0; i < iterations; ++i) { // Пример: отправляем 11 случайных чисел
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
    
    //закрыть открытый объект
    close(shm_id);
    return 0;
}

