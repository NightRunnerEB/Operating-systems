#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/wait.h>   /* for waitpid() */

#define MAXPENDING 2
#define BUFSIZE 1000

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

char *endMessage = "The End";
int clnt1Sock;
int clnt2Sock;

int CreateTCPServerSocket(unsigned short port)
{
    int sock;                        /* socket to create */
    struct sockaddr_in servAddr; /* Local address */

    /* Create socket for incoming connections */
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed");
      
    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(port);              /* Local port */

    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed");

    /* Mark the socket so it will listen for incoming connections */
    if (listen(sock, MAXPENDING) < 0)
        DieWithError("listen() failed");

    printf("Server IP address = %s. Wait...\n", inet_ntoa(servAddr.sin_addr));

    return sock;
}

void HandleTCPClient(int clntSocket)
{
    char echoBuffer[BUFSIZE];        /* Buffer for echo string */
    int recvMsgSize;                    /* Size of received message */

    /* Receive message from client */
    if ((recvMsgSize = recv(clntSocket, echoBuffer, BUFSIZE, 0)) < 0)
        DieWithError("recv() failed");
    
    echoBuffer[recvMsgSize - 2] = '\0';  /* Terminate the string! */

    /* Send received string and receive again until end of transmission */
    while (1)
    {
        printf("Sending: \"%s\" to client 2\n", echoBuffer);

        if (send(clnt2Sock, echoBuffer, recvMsgSize, 0) != recvMsgSize)
            DieWithError("send() failed");
        
        if (strcmp(echoBuffer, endMessage) == 0) {
            return;
        }

        if ((recvMsgSize = recv(clntSocket, echoBuffer, BUFSIZE, 0)) < 0)
            DieWithError("recv() failed");
        
        echoBuffer[recvMsgSize - 2] = '\0';
    }
}


int AcceptTCPConnection(int servSock)
{
    int clntSock;                    /* Socket descriptor for client */
    struct sockaddr_in echoClntAddr; /* Client address */
    unsigned int clntLen;            /* Length of client address data structure */

    /* Set the size of the in-out parameter */
    clntLen = sizeof(echoClntAddr);

    /* Wait for a client to connect */
    if ((clntSock = accept(servSock, (struct sockaddr *) &echoClntAddr,
           &clntLen)) < 0)
        DieWithError("accept() failed");

    /* clntSock is connected to a client! */

    printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

    return clntSock;
}

int main(int argc, char *argv[])
{
    int servSock;                    /* Socket descriptor for server */
    int clntSock;                    /* Socket descriptor for client */
    unsigned short echoServPort;     /* Server port */
    pid_t processID;                 /* Process ID from fork() */
    unsigned int childProcCount = 0; /* Number of child processes */

    if (argc != 2)     /* Test for correct number of arguments */
    {
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }

    echoServPort = atoi(argv[1]);  /* First arg:  local port */

    servSock = CreateTCPServerSocket(echoServPort);
    clnt1Sock = AcceptTCPConnection(servSock);
    clnt2Sock = AcceptTCPConnection(servSock);

    HandleTCPClient(clnt1Sock);

    printf("Server ends his works....\n");

    close(clnt1Sock);
    close(clnt2Sock);
    exit(0);
}
