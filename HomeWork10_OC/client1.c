#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define BUFSIZE 1000


int sock;                        /* Socket descriptor */
char *endMessage = "The End\n";

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void finish_by_signal(int signal)
{
    int stringLen = strlen(endMessage);          /* Determine input length */
    if (send(sock, endMessage, stringLen + 1, 0) != stringLen + 1)
        DieWithError("send() sent a different number of bytes than expected");
    close(sock);
    exit(0);
}


int main(int argc, char *argv[])
{

    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short servPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *echoString;                /* String to send to echo server */
    char buffer[BUFSIZE];     /* Buffer for echo string */
    unsigned int stringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv()
                                        and total bytes read */

    if ((argc < 3))    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n",
               argv[0]);
       exit(1);
    }

    signal(SIGINT, finish_by_signal);
    servIP = argv[1];             /* First arg: server IP address (dotted quad) */
    servPort = atoi(argv[2]);

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port = htons(servPort); /* Server port */

    /* Establish the connection to the server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");

    while(1)
    {
        printf("Enter your message to client 2:");
        fgets(buffer, BUFSIZE, stdin);
        stringLen = strlen(buffer);
        buffer[stringLen] = '\0';
        printf("You entered: %s", buffer);

        if (send(sock, buffer, stringLen + 1 , 0) != stringLen + 1)
            DieWithError("send() sent a different number of bytes than expected");
        
        if (strcmp(buffer, endMessage) == 0) {
            break;
        }
    }

    printf("Sender(client1) ends his work....\n");
    close(sock);
    exit(0);
}
