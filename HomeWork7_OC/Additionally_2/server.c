// server.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>

char gen_object[] = "gen-memory";

void cleanup(int sig) {
    // Код для удаления разделяемой памяти
    if(shm_unlink(gen_object) == -1) {
        perror("shm_unlink cleanup");
    }
    printf("Server cleanup completed\n");
    exit(0); // Завершаем работу сервера
}

int main () {
    signal(SIGINT, cleanup); // Установка обработчика сигнала
    
    int shm_id;
    struct stat mapstat;
    
    printf("Server PID: %d\n", getpid());
    
    //открыть объект
    if ( (shm_id = shm_open(gen_object, O_CREAT|O_RDWR, 0666)) == -1 ) {
      printf("Opening error\n");
      perror("shm_open");
      return 1;
    } else {
      printf("Object is open: name = %s, id = 0x%x\n", gen_object, shm_id);
    }
    
    // Задание размера объекта памяти
    if (-1 != fstat(shm_id, &mapstat) && mapstat.st_size == 0) {
        if (ftruncate(shm_id, sizeof (int)) == -1) {
          perror("ftruncate");
          return 1;
        } else {
          printf("Memory size set and = %lu\n", sizeof (int));
        }
    }
    
    //получить доступ к памяти
    int* data = mmap(0, sizeof(int), PROT_WRITE|PROT_READ, MAP_SHARED, shm_id, 0);
    if (data == (int*)-1 ) {
      printf("Error getting pointer to shared memory\n");
      return 1;
    }
    
    // Ожидаем данные от клиента
    while (1) {
        if (*data != 0) { // Проверяем, есть ли новые данные
            printf("Получено число: %d\n", *data);
            *data = 0; // Сбрасываем флаг данных
        }
        sleep(1); // Чтобы не нагружать процессор
    }
    
    return 0;
}

