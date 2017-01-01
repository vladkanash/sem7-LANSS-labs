//
// Created by vladkanash on 2.11.16.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "engine.h"

static command_holder command_info[COMMAND_COUNT];

void init_commands() {
    command_info[0].type = ECHO;
    command_info[0].simple = false;
    command_info[0].length = sizeof(COMMAND_ECHO) - 1;
    command_info[0].name = COMMAND_ECHO;
    command_info[0].next_state = ECHOING;

    command_info[1].type = DOWNLOAD;
    command_info[1].simple = false;
    command_info[1].length = sizeof(COMMAND_DOWNLOAD) - 1;
    command_info[1].name = COMMAND_DOWNLOAD;
    command_info[1].next_state = START_UPLOADING;

    command_info[2].type = TIME;
    command_info[2].simple = true;
    command_info[2].length = sizeof(COMMAND_TIME) - 1;
    command_info[2].name = COMMAND_TIME;
    command_info[2].next_state = IDLE;

    command_info[3].type = CLOSE;
    command_info[3].simple = true;
    command_info[3].length = sizeof(COMMAND_CLOSE) - 1;
    command_info[3].name = COMMAND_CLOSE;
    command_info[3].next_state = IDLE;
}

server_command get_command(char *buf, const unsigned int nread) {
    server_command command;
    memset(&command, 0, sizeof(server_command));
    command.success = false;

    static char short_end[64];
    static char long_end[64];

    for (int i = 0; i < COMMAND_COUNT; i++) {
        command_holder com = command_info[i];

        memset(short_end, 0, sizeof(short_end));
        memset(long_end, 0, sizeof(long_end));

        strcat(short_end, com.name);
        strcat(long_end, com.name);

        if (com.simple) {
            strcat(short_end, COMMAND_END_1);
            strcat(long_end, COMMAND_END_2);
        }
        //TODO функция побайтной передачи
        //TODO UDP потеря пакетов
        memset(command.text, 0, sizeof(command.text));
        command.type = com.type;
        command.simple = com.simple;
        command.next_state = com.next_state;

        if (strstr(buf, short_end) == buf) {
            command.success = true;
            strcpy(command.text, short_end);
            command.command_length = (size_t) (command.simple ? com.length + 1 : com.length);
            return command;
        } else if (strstr(buf, long_end) == buf) {
            command.success = true;
            strcpy(command.text, long_end);
            command.command_length = (size_t) (command.simple ? com.length + 2 : com.length);
            return command;
        }
    }

    command.success = false;
    if (strstr(buf, COMMAND_END_2) != NULL) {
        command.command_length = strstr(buf, COMMAND_END_2) - buf + 2;
    } else if (strstr(buf, COMMAND_END_1) != NULL) {
        command.command_length = strstr(buf, COMMAND_END_1) - buf + 1;
    } else {
        command.command_length = nread;
    }
    return command;
}

command_response process_command(const server_command *command) {
    command_response result;
    memset(&result, 0, sizeof(result));

    if (!command->success) {
        result.success = false;
        return result;
    }
    result.success = true;
    memset(result.text, 0, sizeof(result.text));

    switch (command->type) {
        case TIME : {
            get_current_time(&result);
            break;
        }
        case CLOSE : {
            sprintf(result.text, "Closing connection...\r\n");
            break;
        }
        case ECHO : {
            break;
        }
        case DOWNLOAD : {
            break;
        }
        default : {
            result.success = false;
            break;
        }
    }
    return result;
}

size_t find_line_ending(const char *buf, const size_t buf_len) {
    int ending_len = 0;
    char* start = strstr(buf, COMMAND_END_2); {
        ending_len = 2;
        if (NULL == start) {
            ending_len = 1;
            start = strstr(buf, COMMAND_END_1);
        }
    }
    if (NULL != start) {
        return start - buf + ending_len;
    } else {
        return buf_len;
    }
}

void get_current_time(command_response* result) {
    unsigned size = 30;
    struct tm *tm;
    time_t t;
    t = time(NULL);
    tm = localtime(&t);
    memset(result->text, 0, size);
    strftime(result->text , size, "%Y-%m-%d %X", tm);
}
