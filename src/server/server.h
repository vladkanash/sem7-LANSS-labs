#include <stdbool.h>
#include "types.h"

#ifndef LANSS_SERVER_H
#define LANSS_SERVER_H


void run_server(struct sockaddr_in *sap);

void input_data(int fd);

void add_client(int fd);

void close_connection(int fd);

void parse_command_end(session_handler* session);

void parse_command_start(session_handler* session);

void start_file_upload(session_handler* session);

void upload_file_part(session_handler* session);

void flush_socket(session_handler* session);

void init_client_session(int fd);

void update_session_state(server_command *command);

void start_file_upload(session_handler* session);

void upload_file_part(session_handler* session);

void stop_server();

#endif //LANSS_SERVER_H
