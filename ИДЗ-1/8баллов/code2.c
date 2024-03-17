#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 5000

void capitalize_consonants(char *str) {
    while (*str) {
        if (isalpha(*str) && !strchr("aeiouAEIOU", *str)) {
            *str = toupper(*str);
        }
        str++;
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
    ssize_t bytes_read = read(pipe1_fd, buffer, BUFFER_SIZE);
    if (bytes_read == -1) {
        perror("read from pipe1");
        exit(EXIT_FAILURE);
    }
    close(pipe1_fd);

    capitalize_consonants(buffer);

    if (write(pipe2_fd, buffer, bytes_read) == -1) {
        perror("write to pipe2");
        exit(EXIT_FAILURE);
    }
    close(pipe2_fd);

    return EXIT_SUCCESS;
}
