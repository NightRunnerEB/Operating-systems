#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>

#define SHM_ID  0x777     // ключ разделяемой памяти
#define PERMS   0666      // права доступа

// коды сообщений
#define MSG_TYPE_EMPTY  0     // сообщение о завершении обмена
#define MSG_TYPE_INT 1     // сообщение о передаче значения
#define MSG_TYPE_FINISH 2     // сообщение о том, что пора завершать обмен

// структура сообщения, помещаемого в разделяемую память
typedef struct {
  int type;
  int val;
} message_t;

