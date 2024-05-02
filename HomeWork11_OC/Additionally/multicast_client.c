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
    struct ip_mreq multicastRequest;
    unsigned short multicastPort;
    char echoString[ECHOMAX+1];
    int recvStringLen;

    if (argc != 3) {
        fprintf(stderr,"Usage: %s <Multicast IP> <Port>\n", argv[0]);
        exit(1);
    }

    char *multicastIP = argv[1];
    multicastPort = atoi(argv[2]);

    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    multicastAddr.sin_port = htons(multicastPort);

    if (bind(sock, (struct sockaddr *) &multicastAddr, sizeof(multicastAddr)) < 0)
        DieWithError("bind() failed");

    multicastRequest.imr_multiaddr.s_addr = inet_addr(multicastIP);
    multicastRequest.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *) &multicastRequest,
                   sizeof(multicastRequest)) < 0)
        DieWithError("setsockopt() failed");

    while (1) {
        if ((recvStringLen = recvfrom(sock, echoString, ECHOMAX, 0, NULL, 0)) < 0)
            DieWithError("recvfrom() failed");

        echoString[recvStringLen] = '\0';
        printf("Received: %s", echoString);
    }

    close(sock);
    exit(0);
}
