#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int fibonacci_plus(int num) {
    if (num <= 1) {
        return num;
    } else {
        return fibonacci_plus(num - 1) + fibonacci_plus(num - 2);
    }
}

int fibonacci_minus(int num) {
    if (num >= -1) {
        return num;
    } else {
        return fibonacci_minus(num + 2) - fibonacci_minus(num + 1);
    }
}

int main(int argc, char *argv[]) {
    int num = atoi(argv[1]);
    pid_t chpid;
    int result;
    chpid = fork();
    if (chpid == 0) {
        if(num < 0) {
            printf("I can't calculate the factorial of a negative number:(\n");
        } else if(num == 0){
            printf("Factorial 0 = 1\n");
        } else {
            int count = num;
            result = num;
            while(count > 1) {
                count -= 1;
                result = result * count;
            }
            printf("Factorial %d = %d\n", num, result);
        }
    } else if (chpid == -1) {
        printf("Error");
    } else {
        if(num > 0) {
            result = fibonacci_plus(num);
        } else {
            result = fibonacci_minus(num);
        }
        printf("The Fibonacci number for %d is %d\n", num, result);
    }
    return 0;
}
