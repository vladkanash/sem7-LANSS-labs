//
// Created by vladkanash on 16.9.16.
//
#include <types.h>
#include <netdb.h>
#include <libgen.h>
#include <sys/time.h>
#include "client.h"

struct timeval timeout;
struct timeval timeout_buf;
fd_set set, buf_set;
int fd_skt, nread;
file_info download_info;
FILE* file;
size_t remaining = 0;
struct timeval t1, t2;
double elapsed;
char buf[BUF_SIZE];
client_state state = CLIENT_IDLE;

char uuid[UUID_LENGTH];

int read_with_timeout(void *input, size_t size, int flags);

void processInput();

int main(int argc, char** argv) {
    struct sockaddr_in sa;
    int portno, result;
    char* address, ip[100];
    ssize_t nread;

    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    if (argc < 3) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    address = argv[1];
    portno = atoi(argv[2]);

    hostname_to_ip(address, ip);

    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t) portno);
    sa.sin_addr.s_addr = inet_addr(ip);
    fd_skt = socket(AF_INET, SOCK_STREAM, 0);

    get_uuid(uuid);

    if (connect(fd_skt, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
       dprintf(STDERR_FILENO, "Can't connect to %s:%d, Exiting...", address, portno);
        exit(-1);
    }

    FD_SET(fd_skt, &set);

    bool running = true;

    while (running) {

        switch(state) {
            case CLIENT_IDLE : {
                processInput();
                break;
            }
            case CLIENT_START_DOWNLOADING : {
                char* file_name = basename(buf);
                file_name[strlen(file_name) - 1] = '\0';

                memset(&download_info, 0, sizeof(download_info));

                read_with_timeout(&download_info, sizeof(download_info), MSG_PEEK);

                if (strcmp(file_name, download_info.name)) {
                    state = CLIENT_IDLE;
                    break;
                }
                read_with_timeout(&download_info, sizeof(download_info), 0);
                file = fopen(download_info.name, "a");
                remaining = download_info.size;
                state = CLIENT_DOWNLOADING;
                gettimeofday(&t1, NULL);
                break;
            }
            case CLIENT_DOWNLOADING: {
                memset(buf, 0, BUF_SIZE);
                nread = read_with_timeout(buf, BUF_SIZE, 0);
                remaining -= nread;
                if (nread == -1 ) {
                    printf("Problem with connection... Cannot download file\n");
                    state = CLIENT_IDLE;
                    break;
                }
                if (remaining <= 0) {
                    gettimeofday(&t2, NULL);
                    elapsed = (t2.tv_sec - t1.tv_sec) * 1000;
                    printf("Download finished. Average speed is %f\n", download_info.size / elapsed);
                    fclose(file);
                    state = CLIENT_IDLE;
                } else {
                    fwrite(buf, sizeof(char), (size_t) nread, file);
                }
                break;
            }
        }
    }

    close(fd_skt);
    exit(EXIT_SUCCESS);
}

void processInput() {
    fgets (buf, BUF_SIZE, stdin);
    send_data(fd_skt, buf, (int) strlen(buf), 0);

    if (strstr(buf, COMMAND_DOWNLOAD) != NULL) {
        state = CLIENT_START_DOWNLOADING;
        return;
    }
    memset(buf, 0, sizeof(buf));

    read_with_timeout(buf, BUF_SIZE, 0);

    if (strlen(buf) > 0) {
        printf("%s\n", buf);
    }
}

int read_with_timeout(void *input, size_t size, int flags) {
    timeout_buf = timeout;
    buf_set = set;
    select(fd_skt + 1, &buf_set, 0, 0, &timeout_buf);
    if (FD_ISSET(fd_skt, &buf_set)) {
        ssize_t nread = recv(fd_skt, input, size, flags);
        return (int) nread;
    }
    return -1;
}

int hostname_to_ip(char * hostname , char* ip) {
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ( (he = gethostbyname( hostname ) ) == NULL) {
        // get the host info
        herror("Cannot get host from this name");
        return 1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++) {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]) );
        return 0;
    }

    return 1;
}