//
// Created by vladkanash on 13.11.16.
//
#include "download_list.h"

static download_handler list[MAX_CLIENTS];
static int max_idx = 0;

void add_download(char *uuid, int file) {
    if (NULL == uuid) {
        return;
    }

    download_handler handler;
    handler.file = file;
    strncpy(handler.uuid, uuid, UUID_LENGTH);
    handler.offset = 0;

    list[max_idx] = handler;
    max_idx++;
}

bool download_exists(char *uuid) {
    for (int i = 0; i < max_idx; i++) {
        if (!strncmp(list[i].uuid, uuid, UUID_LENGTH)) {
            return true;
        }
    }
    return false;
}

download_handler* get_download(char *uuid) {
    for (int i = 0; i < max_idx; i++) {
        if (!strncmp(list[i].uuid, uuid, UUID_LENGTH)) {
            return &(list[i]);
        }
    }
    return NULL;
}

void remove_download(char *uuid) {
    for (int i = 0; i < max_idx; i++) {
        if (!strncmp(list[i].uuid, uuid, UUID_LENGTH)) {

            for (int j = i; j < max_idx; j++) {
                list[j] = list[j + 1];
            }
            max_idx--;
        }
    }
}