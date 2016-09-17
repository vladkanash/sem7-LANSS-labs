//
// Created by vladkanash on 16.9.16.
//

/* A simple server in the internet domain using TCP
   The port number is passed as an argument

   Usage:
        Start this, then do 'nc 127.0.0.1 [port_number]'
        and send ur messages*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>

#include "commands.h"

void process_command(const char *buf, char *out);
void get_current_time(char *out);

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, msgsock, rval;
    socklen_t clilen;
    char buf[256];
    char out[256];
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    struct sockaddr_in serv_addr, cli_addr;

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

    listen(sockfd, SOMAXCONN);

    printf("My process ID : %d\n", getpid());
    printf("Listening to port : %d\n", portno);
    while(1) {
         msgsock = accept(sockfd, 0, 0);
         if (msgsock == -1) {
             perror("accept");
         } else do {
             bzero(buf, sizeof(buf));
             if ((rval = (int) read(msgsock, buf, 1024)) < 0) {
                 perror("reading stream message");
             } else if (rval == 0) {
                 printf("Ending connection\n");
             } else {
                 process_command(buf, out);
                 printf("COMMAND-->%s\n", buf);
                 printf("RESPONSE-->%s\n", out);
                 write(msgsock, out, sizeof(out));
             }
         } while (rval > 0);
         close(msgsock);
     }
     close(sockfd);
}

void process_command(const char *buf, char *out) {
    if (strcmp(buf, COMMAND_TIME) == 0) {
        get_current_time(out);
    } else if (strcmp(buf, COMMAND_ECHO) == 0) {

    } else if (strcmp(buf, COMMAND_CLOSE) == 0) {

    }
}

void get_current_time(char *out) {
    struct tm *tm;
    time_t t;
    t = time(NULL);
    tm = localtime(&t);
    strftime(out, 30, "%F %X\n", tm);
}
