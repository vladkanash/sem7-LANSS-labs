//
// Created by vladkanash on 16.9.16.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  REQUEST "GET / HTTP/1.0\r\n\r\n"

int main(void) {
    struct sockaddr_in sa;
    int fd_skt;
    char buf[1000];
    ssize_t nread;

    sa.sin_family = AF_INET;
    sa.sin_port = htons(80);
    sa.sin_addr.s_addr = inet_addr("216.58.214.206");
    fd_skt = socket(AF_INET, SOCK_STREAM, 0);
    connect(fd_skt, (struct sockaddr*)&sa, sizeof(sa));
    write(fd_skt, REQUEST, sizeof(buf));
    nread = read(fd_skt, buf, sizeof(buf));
    write(STDOUT_FILENO, buf, (size_t) nread);
    close(fd_skt);
    exit(EXIT_SUCCESS);
}