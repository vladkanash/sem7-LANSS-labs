#ifndef LANSS_CLIENT_H
#define LANSS_CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <memory.h>
#include <constants.h>
#include <system_dependent_code.h>
#include <server/engine.h>

bool process_user_input();

void init_download(char *file_path);

void download_file_part();

int hostname_to_ip(char * hostname , char* ip);

void set_socket_timeout();

void process_raw_input();

bool starts_with(const char *str, const char *pre);

#endif //LANSS_CLIENT_H

