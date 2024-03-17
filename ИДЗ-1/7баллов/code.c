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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process: Read from named pipe "pipe1", process data, write to named pipe "pipe2"
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
        exit(EXIT_SUCCESS);
    } else {
        // Parent process: Read from file, write to named pipe "pipe1", read from named pipe "pipe2", write to file
        int input_fd = open(argv[1], O_RDONLY);
        if (input_fd == -1) {
            perror("open input file");
            exit(EXIT_FAILURE);
        }

        int pipe1_fd = open("pipe1", O_WRONLY);
        if (pipe1_fd == -1) {
            perror("open pipe1");
            exit(EXIT_FAILURE);
        }

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = read(input_fd, buffer, BUFFER_SIZE);
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        close(input_fd);

        if (write(pipe1_fd, buffer, bytes_read) == -1) {
            perror("write to pipe1");
            exit(EXIT_FAILURE);
        }
        close(pipe1_fd);

        wait(NULL); // Wait for the child process to finish

        int pipe2_fd = open("pipe2", O_RDONLY);
        if (pipe2_fd == -1) {
            perror("open pipe2");
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_processed = read(pipe2_fd, buffer, BUFFER_SIZE);
        if (bytes_processed == -1) {
            perror("read from pipe2");
            exit(EXIT_FAILURE);
        }
        close(pipe2_fd);

        int output_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            perror("open output file");
            exit(EXIT_FAILURE);
        }

        if (write(output_fd, buffer, bytes_processed) == -1) {
            perror("write to output file");
            exit(EXIT_FAILURE);
        }
        close(output_fd);
    }

    return EXIT_SUCCESS;
}
