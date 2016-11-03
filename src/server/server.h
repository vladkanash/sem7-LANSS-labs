#include <stdbool.h>

#ifndef LANSS_SERVER_H
#define LANSS_SERVER_H


void run_server(struct sockaddr_in *sap);

void input_data(int fd);

void add_client(int fd);

void close_connection(int fd);

#endif //LANSS_SERVER_H