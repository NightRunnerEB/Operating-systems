#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind() */
#include <arpa/inet.h>  /* for sockaddr_in, inet_aton() */
#include <stdlib.h>     /* for atoi(), exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define ECHOMAX 255     /* Longest string to receive */

void DieWithError(char *errorMessage) {
    perror(errorMessage);
    exit(1);
}

int main(int argc, char *argv[]) {
    int sock;                         /* Socket */
    struct sockaddr_in broadcastAddr; /* Broadcast address */
    struct sockaddr_in fromAddr;      /* Source address of echo */
    unsigned short broadcastPort;     /* Port */
    unsigned int fromSize;            /* In-out of address size for recvfrom() */
    char echoString[ECHOMAX+1];       /* Buffer for receiving echoed string */
    int recvStringLen;                /* Length of received string */

    if (argc != 2) { /* Test for correct number of parameters */
        fprintf(stderr,"Usage: %s <Port>\n", argv[0]);
        exit(1);
    }

    broadcastPort = atoi(argv[1]); /* First arg:  broadcast port */

    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

    /* Construct bind structure */
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));   /* Zero out structure */
    broadcastAddr.sin_family = AF_INET;                 /* Internet address family */
    broadcastAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
    broadcastAddr.sin_port = htons(broadcastPort);      /* Broadcast port */

    /* Bind to the broadcast port */
    if (bind(sock, (struct sockaddr *) &broadcastAddr, sizeof(broadcastAddr)) < 0)
        DieWithError("bind() failed");

    for (;;) /* Run forever */
    {
        /* Receive a single datagram from the server */
        fromSize = sizeof(fromAddr);
        if ((recvStringLen = recvfrom(sock, echoString, ECHOMAX, 0,
             (struct sockaddr *) &fromAddr, &fromSize)) < 0)
            DieWithError("recvfrom() failed");

        echoString[recvStringLen] = '\0'; /* Terminate the string! */
        printf("Received: %s", echoString); /* Print the received string */
    }

    /* NOT REACHED */
    close(sock);
    exit(0);
}
