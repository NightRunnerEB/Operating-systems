#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define ECHOMAX 255

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in multicastAddr;
    char *multicastIP;
    unsigned short multicastPort;
    char echoString[ECHOMAX];
    
    if (argc != 3) {
        fprintf(stderr,"Usage: %s <Multicast IP> <Port>\n", argv[0]);
        exit(1);
    }

    multicastIP = argv[1];
    multicastPort = atoi(argv[2]);

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = inet_addr(multicastIP);
    multicastAddr.sin_port = htons(multicastPort);

    while (1) {
        printf("Enter message to multicast: ");
        fgets(echoString, ECHOMAX, stdin);

        if (sendto(sock, echoString, strlen(echoString), 0,
            (struct sockaddr *)&multicastAddr, sizeof(multicastAddr)) != strlen(echoString))
            DieWithError("sendto() sent a different number of bytes than expected");

        if (strcmp(echoString, "exit\n") == 0)
            break;
    }

    close(sock);
    exit(0);
}
