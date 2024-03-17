#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 128

void process_data(char *str, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        if (isalpha(str[i]) && !strchr("aeiouAEIOU", str[i])) {
            str[i] = toupper(str[i]);
        }
    }
}

int main() {
    int pipe1_fd = open("pipe1", O_RDONLY);
    if (pipe1_fd == -1) {
        perror("open pipe1");
        exit(EXIT_FAILURE);
    }

    int pipe2_fd = open("pipe2", O_WRONLY);
    if (pipe2_fd == -1) {
        perror("open pipe2");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(pipe1_fd, buffer, BUFFER_SIZE)) > 0) {
        process_data(buffer, bytes_read);

        if (write(pipe2_fd, buffer, bytes_read) == -1) {
            perror("write to pipe2");
            exit(EXIT_FAILURE);
        }
    }

    close(pipe1_fd);
    close(pipe2_fd);

    return EXIT_SUCCESS;
}
