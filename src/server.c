//
// Created by vladkanash on 7.10.16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <stdbool.h>

#include "commands.h"

#define BUF_SIZE 256

int fd_hwm = 0;
fd_set set;

void echo_message(char *in_buf, char *out_buf);
void process_command(int fd, char *in_buf, char *out_buf);

void get_current_time(char *out) {
    struct tm *tm;
    time_t t;
    t = time(NULL);
    tm = localtime(&t);
    strftime(out, 30, "%F %X\n", tm);
}

bool startsWith(const char *pre, const char *str)
{
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}

void run_server(struct sockaddr_in *sap) {
    int fd_skt, fd_client, fd;
    char in_buf[BUF_SIZE];
    char out_buf[BUF_SIZE];
    memset(in_buf, 0, sizeof(in_buf));
    memset(out_buf, 0, sizeof(out_buf));
    fd_set read_set;

    fd_skt = socket(AF_INET, SOCK_STREAM, 0);

    if (bind(fd_skt, (struct sockaddr*) sap, sizeof(*sap)) < 0) {
        perror("error while binding socket");
        exit(-1);
    }

    if (listen(fd_skt, SOMAXCONN) < 0) {
        perror("error while trying to listen");
        exit(-1);
    }

    if (fd_skt > fd_hwm) {
        fd_hwm = fd_skt;
    }

    FD_ZERO(&set);
    FD_SET(fd_skt, &set);

    bool running = true;
    while (running) {
        read_set = set;
        select(fd_hwm + 1, &read_set, NULL, NULL, NULL);
        for (fd = 0; fd <= fd_hwm; fd++) {
            if (FD_ISSET(fd, &read_set)) {
                if (fd == fd_skt) {
                    fd_client = accept(fd_skt, NULL, 0);
                    FD_SET(fd_client, &set);
                    if (fd_client > fd_hwm) {
                        fd_hwm = fd_client;
                    }
                } else {
                    ssize_t nread = read(fd, in_buf, sizeof(in_buf));
                    process_command(fd, in_buf, out_buf);
                }
            }
        }
    }
    close(fd_skt);
    return;
}

void process_command(int fd, char *in_buf, char *out_buf) {
    if (strcmp(in_buf, COMMAND_TIME) == 0) {
        get_current_time(out_buf);
    } else if (startsWith(COMMAND_ECHO, in_buf)) {
        echo_message(in_buf, out_buf);
    } else if (strcmp(in_buf, COMMAND_CLOSE) == 0) {
        FD_CLR(fd, &set);
        if (fd == fd_hwm) {
            fd_hwm--;
        }
        memset(out_buf, 0, BUF_SIZE);
        close(fd);
    } else {
        memset(out_buf, 0, BUF_SIZE);
    }
    memset(in_buf, 0, BUF_SIZE);
    write(fd, out_buf, BUF_SIZE);
}

void echo_message(char *in_buf, char *out_buf) {
    memcpy(out_buf, in_buf, BUF_SIZE);
    memmove(out_buf, out_buf+5, BUF_SIZE);
}

