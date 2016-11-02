#include "../types.h"

#ifndef LANSS_ENGINE_H
#define LANSS_ENGINE_H

void get_current_time(command_response* result);

void echo_message(char *in_buf, char *out_buf);

command_response process_command(server_command command);

void download_file(char *buf, char *out_buf, int i);

bool parse_command(char *input, char command_type[5], int command_len, server_command* command);

server_command get_command(char *buf);

void cleanup(char *in_buf, char *out_buf, server_command *command, command_response *response);

bool startsWith(const char *pre, const char *str);

#endif //LANSS_ENGINE_H
