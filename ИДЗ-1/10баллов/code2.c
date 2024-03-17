#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <mqueue.h>
#include <string.h>

#define BUFFER_SIZE 128
#define QUEUE_NAME "/my_queue"

void process_data(char *str, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (isalpha(str[i]) && !strchr("aeiouAEIOU", str[i])) {
            str[i] = toupper(str[i]);
        }
    }
}

int main() {
    mqd_t mq = mq_open(QUEUE_NAME, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = mq_receive(mq, buffer, BUFFER_SIZE, NULL)) > 0) {
        process_data(buffer, bytes_read);

        if (mq_send(mq, buffer, bytes_read, 0) == -1) {
            perror("mq_send");
            exit(EXIT_FAILURE);
        }
    }

    mq_close(mq);
    mq_unlink(QUEUE_NAME);

    return EXIT_SUCCESS;
}
