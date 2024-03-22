#include<stdio.h>
#include<unistd.h>
#include<sys/shm.h>
#include<stdlib.h>
#include "message.h"

int main () {
  int shmid;
  message_t *msg_p;

  if ((shmid = shmget (SHM_ID, sizeof (message_t), PERMS | IPC_CREAT)) < 0) {
    printf("server: can not create shared memory segment");
    exit(-1);
  }

  if ((msg_p = (message_t *) shmat (shmid, 0, 0)) == NULL) {
    printf("server: shared memory attach error");
    exit(-1);
  }

  msg_p->type = MSG_TYPE_EMPTY;
  while (1) {
    if (msg_p->type != MSG_TYPE_EMPTY) {
      // обработка сообщения
      if (msg_p->type == MSG_TYPE_INT) {
        printf ("Получено число: %d\n", msg_p->val);
      } else if (msg_p->type == MSG_TYPE_FINISH) {
        break;
      }
      msg_p->type = MSG_TYPE_EMPTY; // сообщение обработано
    }
  }

  // отсоединить и удалить сегмент
  shmdt (msg_p);
  if (shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
    printf("server: shared memory remove error");
    exit(-1);
  }
  exit(0);
}
