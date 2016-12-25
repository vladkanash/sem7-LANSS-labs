#include "../types.h"

#ifndef LANSS_ENGINE_H
#define LANSS_ENGINE_H

void init_commands();

void get_current_time(command_response* result);

command_response process_command(const server_command* command);

bool parse_command(char *input, server_command* command);

server_command get_command(char *buf, const unsigned int nread);

bool get_long_command(char* buf, server_command* command);

size_t find_line_ending(const char *buf, const size_t buf_len);

void check_download_arguments(server_command *session);

#endif //LANSS_ENGINE_H
