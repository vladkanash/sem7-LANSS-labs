//
// Created by vladkanash on 16.9.16.
//
#include <types.h>
#include <netdb.h>
#include <libgen.h>
#include <sys/time.h>
#include "client.h"

static double elapsed;
static struct timeval t1, t2;

void get_size_str(double elapsed, char* str);
void print_speed_info();

static fd_set set;
static int fd_skt;

static file_info download_info;
static FILE* file;
static size_t remaining = 0;

static client_state state = CLIENT_IDLE;
static char uuid[UUID_LENGTH];

int main(int argc, char** argv) {
    struct sockaddr_in sa;
    int port_number;
    char* address, ip[100];

    if (argc < 3) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    address = argv[1];
    port_number = atoi(argv[2]);

    if (-1 == hostname_to_ip(address, ip)) {
        fprintf(stderr, "ERROR, cant read host address\n");
    }

    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t) port_number);
    sa.sin_addr.s_addr = inet_addr(ip);
    fd_skt = socket(AF_INET, SOCK_STREAM, 0);

    get_uuid(uuid);

    if (connect(fd_skt, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
       fprintf(stderr, "Can't connect to %s:%d, Exiting...", address, port_number);
        exit(-1);
    }

    FD_SET(fd_skt, &set);

    set_socket_timeout();
    send_data(fd_skt, uuid, UUID_LENGTH, 0);

    bool running = true;
    while (running) {
        switch(state) {
            case CLIENT_IDLE : {
                running = process_user_input();
                break;
            }
            case CLIENT_DOWNLOADING: {
                download_file_part();
                break;
            }
            case CLIENT_ECHOING: {
                process_raw_input();
                break;
            }
        }
    }
    close(fd_skt);
    exit(EXIT_SUCCESS);
}

void set_socket_timeout() {
    struct timeval tv;
    tv.tv_sec = CONN_TIMEOUT;
    tv.tv_usec = 0;  // Not init'ing this can cause strange errors
    setsockopt(fd_skt, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
}

void download_file_part() {
    static ssize_t nread;
    static char buf[CHUNK_SIZE];
    memset(buf, 0, CHUNK_SIZE);

    nread = recv(fd_skt, buf, CHUNK_SIZE, 0);
    if (nread == -1) {
        printf("Problem with connection... Cannot download file\n");
        state = CLIENT_IDLE;
    } else {
        fwrite(buf, sizeof(char), (size_t) nread, file);
    }

    remaining -= nread;

    if (remaining <= 0) {
        gettimeofday(&t2, NULL);
        elapsed = (double) (t2.tv_usec - t1.tv_usec) / 1000000 +
                  (double) (t2.tv_sec - t1.tv_sec);

        print_speed_info();

        fclose(file);
        state = CLIENT_IDLE;
    }
}

void print_speed_info() {
    char size_str[12];
    memset(size_str, 0, sizeof(size_str));
    get_size_str(download_info.size / elapsed, size_str);
    printf("\nDownload finished. Average speed is %s \n", size_str);
}

void get_size_str(double elapsed, char* str) {
    if (elapsed > 1000000000) {
        sprintf(str, "%.3f Gbps", elapsed / 1000000000);
    } else if (elapsed > 1000000) {
        sprintf(str, "%.3f Mbps", elapsed / 1000000);
    } else if (elapsed > 1000) {
        sprintf(str, "%.3f Kbps", elapsed / 1000);
    } else sprintf(str, "%f bps", elapsed);
}

void init_download(char *file_path) {
    static ssize_t nread;
    char* file_name = basename(file_path);
    file_name[strlen(file_name) - 1] = '\0';

    memset(&download_info, 0, sizeof(download_info));

    nread = recv(fd_skt, &download_info, sizeof(file_info), 0);
    if (nread != sizeof(file_info)) {
        printf("Problem with connection... Cannot download file\n");
        state = CLIENT_IDLE;
    }

    if (strcmp(file_name, download_info.name)) {
        state = CLIENT_IDLE;
        return;
    }

    if (strlen(download_info.comment) > 0) {
        printf("%s\n", download_info.comment);
    }

    if (download_info.size == 0) {
        state = CLIENT_IDLE;
        return;
    }

    file = fopen(download_info.name, "a");
    remaining = download_info.size;
    state = CLIENT_DOWNLOADING;
    gettimeofday(&t1, NULL);
}

bool process_user_input() {
    static ssize_t nread;
    static char input_buf[BUF_SIZE];
    static command_response response;
    bool command_found = false;

    while (strstr(input_buf, "\n") == NULL) {
        memset(input_buf, 0, BUF_SIZE);
        fgets(input_buf, BUF_SIZE, stdin);  //read user input

        if (starts_with(input_buf, COMMAND_ECHO)) {
            state = CLIENT_ECHOING;
            command_found = true;
        }

        send_data(fd_skt, input_buf, (int) strlen(input_buf), 0); //sending input to server
    }

    memset(&response, 0, sizeof(command_response));
    nread = recv(fd_skt, &response, sizeof(command_response), 0); //waiting for the response
    if (nread != sizeof(command_response)) {
        fprintf(stderr, "ERROR. Problems with connection.");
        return false;
    }

    if (response.success && strlen(response.text) > 0) {
        printf("%s\n", response.text);
    }

    if (starts_with(input_buf, COMMAND_DOWNLOAD)
        && response.success
        && !command_found) {

        init_download(input_buf);
        return true;
    }

    if (starts_with(input_buf, COMMAND_CLOSE) && !command_found) {
        return false;
    }

    memset(input_buf, 0, BUF_SIZE);
    return true;
}

void process_raw_input() {
    static ssize_t nread;
    static char buf[BUF_SIZE];
    memset(&buf, 0, BUF_SIZE);

    nread = recv(fd_skt, &buf, BUF_SIZE, 0);
    if (nread == -1) {
        printf("Problem with connection... Cannot download file\n");
        state = CLIENT_IDLE;
    }

    printf("%s", buf);

    if (strstr(buf, "\n") != NULL) {
        state = CLIENT_IDLE;
    }
}

int hostname_to_ip(char * hostname , char* ip) {
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ((he = gethostbyname(hostname)) == NULL) {
        herror("Cannot get host from this name");
        return -1;
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++) {
        //Return the first one;
        strcpy(ip , inet_ntoa(*addr_list[i]));
        return 0;
    }

    return -1;
}

bool starts_with(const char *str, const char *pre)
{
    size_t lenpre = strlen(pre),
            lenstr = strlen(str);
    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
}