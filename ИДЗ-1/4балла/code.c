#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 5000

void capitalize_consonants(char *str) {
    while (*str) {
        if (isalpha(*str) && !strchr("aeiouAEIOU", *str)) {
            *str = toupper(*str);
        }
        str++;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid1 = fork();
    if (pid1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid1 == 0) {
        // Child process 1: Read from file and write to pipe1
        close(pipe1[0]);
        int input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1) {
            perror("open input file");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(input_fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        close(input_fd);

        if (write(pipe1[1], buffer, bytes_read) == -1) {
            perror("write to pipe1");
            exit(EXIT_FAILURE);
        }
        close(pipe1[1]);
        exit(EXIT_SUCCESS);
    }

    pid_t pid2 = fork();
    if (pid2 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid2 == 0) {
        // Child process 2: Read from pipe1, process data, write to pipe2
        close(pipe1[1]);
        close(pipe2[0]);

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(pipe1[0], buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("read from pipe1");
            exit(EXIT_FAILURE);
        }
        close(pipe1[0]);

        capitalize_consonants(buffer);

        if (write(pipe2[1], buffer, bytes_read) == -1) {
            perror("write to pipe2");
            exit(EXIT_FAILURE);
        }
        close(pipe2[1]);
        exit(EXIT_SUCCESS);
    }

    // Parent process: Read from pipe2 and write to file
    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[1]);

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(pipe2[0], buffer, BUFFER_SIZE);
    if (bytes_read == -1) {
        perror("read from pipe2");
        exit(EXIT_FAILURE);
    }
    close(pipe2[0]);

    int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        perror("open output file");
        exit(EXIT_FAILURE);
    }

    if (write(output_fd, buffer, bytes_read) == -1) {
        perror("write to output file");
        exit(EXIT_FAILURE);
    }
    close(output_fd);

    return 0;
}

