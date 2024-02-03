#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int fibonacci(int num) {
    if (num <= 1) {
        return num;
    } else {
        return fibonacci(num - 1) + fibonacci(num - 2);
    }
}

int main(int argc, char *argv[]) {
    int num = atoi(argv[1]);
    pid_t chpid;
    chpid = fork();
    if (chpid == 0) {
        int count = num;
        int result = num;
        while(count > 1) {
            count -= 1;
            result = result * count;
        }
        printf("Factorial %d = %d\n", num, result);
    } else if (chpid == -1) {
        printf("Error");
    } else {
        int result = fibonacci(num);
        printf("The Fibonacci number for %d is %d\n", num, result);
    }
    return 0;
}
