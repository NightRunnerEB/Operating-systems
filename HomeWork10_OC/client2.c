#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

#define MAXPENDING 2
#define BUFSIZE 1000

char *endMessage = "The End";

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}


int main(int argc, char **argv){
    struct sockaddr_in echoServAddr; /* Echo server address */
    unsigned short servPort;     /* Echo server port */
    char *servIP;                    /* Server IP address (dotted quad) */
    char *echoString;                /* String to send to echo server */
    char buffer[BUFSIZE];     /* Buffer for echo string */
    unsigned int stringLen;      /* Length of string to echo */
    int bytesRcvd, totalBytesRcvd;   /* Bytes read in single recv()
                                        and total bytes read */
    int sock;                        /* Socket descriptor */

    if ((argc < 3))    /* Test for correct number of arguments */
    {
       fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n",
               argv[0]);
       exit(1);
    }

    servIP = argv[1];             /* First arg: server IP address (dotted quad) */
    servPort = atoi(argv[2]);

    /* Create a reliable, stream socket using TCP */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
    
    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
    echoServAddr.sin_family      = AF_INET;             /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    echoServAddr.sin_port        = htons(servPort); /* Server port */

    /* Establish the connection to the echo server */
    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0)
        DieWithError("connect() failed");
    
    printf("I'm ready to receive information...\n");

    while (1)
    {
        /* Receive up to the buffer size (minus 1 to leave space for
           a null terminator) bytes from the sender */
        if ((bytesRcvd = recv(sock, buffer, BUFSIZE - 1, 0)) <= 0)
            DieWithError("recv() failed or connection closed prematurely");
        buffer[bytesRcvd] = '\0';  /* Terminate the string! */
        printf("Client 2 received: %s\n", buffer);      /* Print the echo buffer */
        if(strcmp(buffer, endMessage) == 0){
            break;
        }
    }
    printf("Receiver(client2) ends his work...\n");
    close(sock);
    exit(0);
}
