#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind() */
#include <arpa/inet.h>  /* for sockaddr_in, inet_aton() */
#include <stdlib.h>     /* for atoi(), exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define ECHOMAX 255     /* Longest string to broadcast */

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast address */
    unsigned short broadcastPort;     /* Broadcast port */
    char *broadcastIP;                /* IP broadcast address */
    char echoString[ECHOMAX];         /* String to broadcast */
    int broadcastPermission;          /* Socket permission to broadcast */

    if (argc != 3) { /* Test for correct number of parameters */
        fprintf(stderr,"Usage:  %s <IP Address> <Port>\n", argv[0]);
        exit(1);
    }

    broadcastIP = argv[1];            /* First arg:  broadcast IP address */
    broadcastPort = atoi(argv[2]);    /* Second arg: broadcast port */

    /* Create socket for sending datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Set socket to allow broadcast */
    broadcastPermission = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcastPermission,
          sizeof(broadcastPermission)) < 0)
        DieWithError("setsockopt() failed");

    /* Construct local address structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = inet_addr(broadcastIP); /* Broadcast IP address */
    broadcastAddr.sin_port = htons(broadcastPort);      /* Broadcast port */

    while (1) {
        printf("Enter message to broadcast: ");
        fgets(echoString, ECHOMAX, stdin);

        /* Broadcast echoString to clients */
        if (sendto(sock, echoString, strlen(echoString), 0,
             (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) != strlen(echoString))
            DieWithError("sendto() sent a different number of bytes than expected");

        if (strcmp(echoString, "exit\n") == 0)  /* Exit on typing "exit" */
            break;
    }

    close(sock);
    exit(0);
}
