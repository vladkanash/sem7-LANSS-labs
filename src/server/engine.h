#include "../types.h"

#ifndef LANSS_ENGINE_H
#define LANSS_ENGINE_H

void init_commands();

void get_current_time(command_response* result);

command_response process_command(client_session command);

bool parse_command(char *input, client_session* command);

client_session get_command(char *buf);

bool get_long_command(char* buf, client_session *command);

size_t find_line_ending(char *buf);

#endif //LANSS_ENGINE_H
