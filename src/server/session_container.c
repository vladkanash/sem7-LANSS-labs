//
// Created by vladkanash on 19.11.16.
//
#include "session_container.h"

static session_handler session_list[MAX_CLIENTS];
static int max_idx = 0;

session_handler* get_session(const char* uuid) {
    if (NULL == uuid) {
        return NULL;
    }

    for (int i = 0; i < max_idx; i++) {
        if (!memcmp(session_list[i].uuid, uuid, UUID_LENGTH)) {
            return &(session_list[i]);
        }
    }
    return NULL;
}

session_handler* get_session_by_fd(const int fd) {
    if (0 == fd) {
        return NULL;
    }

    for (int i = 0; i < max_idx; i++) {
        if (fd == session_list[i].fd) {
            return &(session_list[i]);
        }
    }
    return NULL;
}

bool put_session(session_handler *new_session) {
    if (NULL == new_session) {
        return false;
    }

    memcpy(&session_list[max_idx], new_session, sizeof(session_handler));
    max_idx++;
    return true;
}

bool remove_session(const char* uuid) {
    for (int i = 0; i < max_idx; i++) {
        if (!memcmp(session_list[i].uuid, uuid, UUID_LENGTH)) {

            for (int j = i; j < max_idx; j++) {
                session_list[j] = session_list[j + 1];
            }
            max_idx--;
            return true;
        }
    }
    return false;
}

void clear_all() {
    memset(session_list, 0, sizeof(session_handler) * MAX_CLIENTS);
    max_idx = 0;
}