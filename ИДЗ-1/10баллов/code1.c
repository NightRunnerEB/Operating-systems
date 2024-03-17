#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <mqueue.h>

#define BUFFER_SIZE 128
#define QUEUE_NAME "/my_queue"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    mqd_t mq = mq_open(QUEUE_NAME, O_WRONLY | O_CREAT, 0644, NULL);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(EXIT_FAILURE);
    }

    int input_fd = open(argv[1], O_RDONLY);
    if (input_fd == -1) {
        perror("open input file");
        exit(EXIT_FAILURE);
    }

    int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("open output file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(input_fd, buffer, BUFFER_SIZE)) > 0) {
        if (mq_send(mq, buffer, bytes_read, 0) == -1) {
            perror("mq_send");
            exit(EXIT_FAILURE);
        }

        if (mq_receive(mq, buffer, BUFFER_SIZE, NULL) == -1) {
            perror("mq_receive");
            exit(EXIT_FAILURE);
        }

        if (write(output_fd, buffer, bytes_read) == -1) {
            perror("write to output file");
            exit(EXIT_FAILURE);
        }
    }

    mq_close(mq);
    mq_unlink(QUEUE_NAME);
    close(input_fd);
    close(output_fd);

    return EXIT_SUCCESS;
}
