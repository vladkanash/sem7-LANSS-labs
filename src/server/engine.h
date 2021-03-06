#include "../types.h"

#ifndef LANSS_ENGINE_H
#define LANSS_ENGINE_H

void init_commands();

void get_current_time(command_response* result);

command_response process_command(server_command command);

bool parse_command(char *input, server_command* command);

server_command get_command(char *buf);

bool get_long_command(char* buf, server_command *command);

size_t find_line_ending(char *buf);

#endif //LANSS_ENGINE_H
