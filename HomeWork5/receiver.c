#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

volatile int bit_count = 0;
volatile int received_number = 0;
volatile int sender_pid = 0; // Храним pid передатчика
volatile bool sending_ended = false;

void handle_sigusr1(int sig) {
    received_number = (received_number << 1) | 1;
    bit_count++;
    kill(sender_pid, SIGUSR1); // Увеломляем передатчика, что бит обработан
}

void handle_sigusr2(int sig) {
    received_number = (received_number << 1) | 0;
    bit_count++;
    kill(sender_pid, SIGUSR1); // Увеломляем передатчика, что бит обработан
}

void handle_end_of_sending(int sig) {
    sending_ended = true;
}

int main() {
    printf("Receiver PID: %d\n", getpid());
    printf("Введите Sender PID: ");
    scanf("%d", &sender_pid); // Читаем pid передатчика

    // Устанавливаем обработчики сигналов
    signal(SIGUSR1, handle_sigusr1);
    signal(SIGUSR2, handle_sigusr2);
    signal(SIGINT, handle_end_of_sending);

    while (!sending_ended) {
        pause(); // Ждем сигнала завершения SIGINT
    }

    printf("Полученное число: %d\n", received_number);

    return 0;
}
