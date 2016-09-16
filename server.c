//
// Created by vladkanash on 16.9.16.
//

/* A simple server in the internet domain using TCP
   The port number is passed as an argument

   Usage:
        After run this, 'nc 127.0.0.1 [port_number]'
        and send ur message*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    portno = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        error("ERROR opening socket");
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons((uint16_t) portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("ERROR on binding");
    }

    printf("My process ID : %d\n", getpid());
    listen(sockfd, SOMAXCONN);

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) {
        error("ERROR on accept");
    }

    bzero(buffer,256);
    n = (int) read(newsockfd, buffer, 255);
    if (n < 0) {
        error("ERROR reading from socket");
    }
    printf("Here is the message: %s\n",buffer);
    n = (int) write(newsockfd, "I got your message", 18);
    if (n < 0) {
        error("ERROR writing to socket");
    }
    close(newsockfd);
    close(sockfd);
    return 0;
}
