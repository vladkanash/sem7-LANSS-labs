//
// Created by vladkanash on 19.11.16.
//
#include <string.h>
#include "types.h"

#ifndef LANSS_SESSION_CONTAINER_H
#define LANSS_SESSION_CONTAINER_H

    session_handler* get_session(const char* uuid);
    session_handler* get_session_by_fd(const int fd);
    bool put_session(session_handler *new_session);
    bool remove_session(const char* uuid);
    void clear_all();

#endif //LANSS_SESSION_CONTAINER_H
