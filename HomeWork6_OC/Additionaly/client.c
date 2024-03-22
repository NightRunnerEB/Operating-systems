#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "message.h"

int main () {
  int num;
  int shmid;            // идентификатор разделяемой памяти
  message_t *msg_p;     // адрес сообщения в разделяемой памяти
  int iterations = 7;
    
  // получение доступа к сегменту разделяемой памяти
    shmid = shmget(SHM_ID, sizeof (message_t), PERMS | IPC_CREAT);
    printf("shm_id = %d\n", shmid);
    if(shmid < 0){
      printf("shmget()");
      exit(-1);
    }

  // получение адреса сегмента
    if ((msg_p = (message_t *) shmat (shmid, 0, 0)) == NULL) {
      printf("client: shared memory attach error");
      exit(-1);
    }

  // Организация передачи сообщений
    while(iterations){
      num = random() % 1000;
      msg_p->val = num;
      msg_p->type = MSG_TYPE_INT;

      while(msg_p->type != MSG_TYPE_EMPTY){} // ожидаем сигнала
      --iterations;
    }

    msg_p->type = MSG_TYPE_FINISH; // завершить работу
    shmdt (msg_p); // удалить сегмент
    return 0;
  }

