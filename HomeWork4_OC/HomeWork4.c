#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 64

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Files: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    int inputFd, outputFd;
    ssize_t bytesRead, bytesWritten;
    struct stat statInfo;

    inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1) {
        perror("Error opening input file");
        return 1;
    }

    outputFd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (outputFd == -1) {
        perror("Error opening output file");
        return 1;
    }
    
    /* Используется для получения информации о файле, на который указывает файловый дескриптор inputFd, и сохранения этой информации в структуре statInfo. */
    fstat(inputFd, &statInfo);
    
    while ((bytesRead = read(inputFd, buffer, BUFFER_SIZE)) > 0) {
        bytesWritten = write(outputFd, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            perror("Error writing to output file");
            return 1;
        }
    }

    if (bytesRead == -1) {
        perror("Error reading input file");
        return 1;
    }

    if (statInfo.st_mode & S_IXUSR) {
        fchmod(outputFd, statInfo.st_mode);
    }

    close(inputFd);
    close(outputFd);

    printf("File copied successfully!\n");

    return 0;
}
