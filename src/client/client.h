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

void print_speed_info(off_t downloaded, size_t total);

bool process_user_input(file_info* download_info);

void init_download(char *file_path, file_info* download_info);

void download_file_part(file_info* download_info);

int hostname_to_ip(char * hostname , char* ip);

void set_socket_timeout();

void process_raw_input();

bool open_connection();

void reopen_socket();

#endif //LANSS_CLIENT_H

