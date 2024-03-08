#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

volatile int bit_received = 0;

void handle_bit(int sig) {
    bit_received = 1;
}

int main() {
    printf("Sender PID: %d\n", getpid());

    int receiver_pid;
    printf("Введите Receiver PID: ");
    scanf("%d", &receiver_pid);

    signal(SIGUSR1, handle_bit);  // Устанавливаем обработчик сигнала SIGUSR1

    int number;
    printf("Введите целое число: ");
    scanf("%d", &number);

    for (int i = 31; i >= 0; i--) {
        bit_received = 0;
        int bit = (number >> i) & 1;
        if (bit) {
            kill(receiver_pid, SIGUSR1); // Увеломляем приемник, что бит равен 1
        }
        else {
            kill(receiver_pid, SIGUSR2); // Увеломляем приемник, что бит равен 0
        }
        while (!bit_received) {
            pause(); // Ждем сигнала подтверждающего получения бита
        }
    }

    kill(receiver_pid, SIGINT); // Уведомляем приемник, что все биты отправлены

    return 0;
}
