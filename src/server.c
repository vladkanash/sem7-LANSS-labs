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
    int fd_skt, fd_client, fd_hwm = 0, fd;
    char buf[256];
    char out[256];
    memset(buf, 0, sizeof(buf));
    memset(out, 0, sizeof(out));
    fd_set set, read_set;
    ssize_t  nread;

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
                nread = read(fd, buf, sizeof(buf));
                    //process message
                    if (strcmp(buf, COMMAND_TIME) == 0) {
                        get_current_time(out);
                    } else if (startsWith(COMMAND_ECHO, buf)) {
                        memcpy(out, buf, sizeof(buf));
                        memmove(out, out+5, sizeof(out));
                    } else if (strcmp(buf, COMMAND_CLOSE) == 0 || nread == 0) {
                        FD_CLR(fd, &set);
                        if (fd == fd_hwm) {
                            fd_hwm--;
                        }
                        memset(out, 0, sizeof(out));
                        close(fd);
                    } else {
                        memset(out, 0, sizeof(out));
                    }
                    memset(buf, 0, sizeof(buf));
                    write(fd, out, sizeof(out));
                }
            }
        }
    }
    close(fd_skt);
    return;
}

