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

#include "message.h"

const char shar_object[] = "posix-shar-object";

void sys_err (char *msg) {
    puts (msg);
    exit (1);
}

int main () {
    int shmid;            // дескриптор разделяемой памяти
    message_t *msg_p;     // адрес сообщения в разделяемой памяти
    int num;
    int iterations = 11;
    
    if ( (shmid = shm_open(shar_object, O_CREAT|O_RDWR, 0666)) == -1 ) {
        perror("shm_open");
        sys_err ("client: object is already open");
    } else {
        printf("Object is open: name = %s, id = 0x%x\n", shar_object, shmid);
    }
    
    srand(time(NULL));
    //получить доступ к памяти
    msg_p = mmap(0, sizeof (message_t), PROT_WRITE|PROT_READ, MAP_SHARED, shmid, 0);
    if (msg_p == (message_t*)-1 ) {
        perror("mmap");
        sys_err ("client: incorrect memory access");
    }
    
    // Организация передачи сообщений
    while(iterations){
        num = random() % 1000;
        msg_p->val = num;
        msg_p->type = MSG_TYPE_INT;
        printf ("Отправлено число: %d\n", msg_p->val);

        while(msg_p->type != MSG_TYPE_EMPTY){} // ожидаем сигнала
        --iterations;
    }
    
    msg_p->type = MSG_TYPE_FINISH; // завершить работу
    //закрыть открытый объект
    close(shmid);
    return 0;
}

