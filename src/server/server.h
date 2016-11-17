#include <stdbool.h>

#ifndef LANSS_SERVER_H
#define LANSS_SERVER_H


void run_server(struct sockaddr_in *sap);

void input_data(int fd);

void add_client(int fd);

void close_connection(int fd);

void parse_command_end(int fd);

void parse_command_start(int fd);

void start_file_upload(int fd);

void upload_file_part(int fd);

void stop_server();

#endif //LANSS_SERVER_H
