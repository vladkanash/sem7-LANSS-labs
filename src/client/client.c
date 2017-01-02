//
// Created by vladkanash on 16.9.16.
//
#include <types.h>
#include <netdb.h>
#include <libgen.h>
#include <sys/time.h>

#include "client.h"
#include "util.h"

static struct timeval t1;
static struct sockaddr_in sa;

static int fd_skt;
static FILE* file;
static client_state state = CLIENT_IDLE;
static char uuid[UUID_LENGTH];
static file_info download_info;

int main(int argc, char** argv) {
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

    get_uuid(uuid);

    sa.sin_family = AF_INET;
    sa.sin_port = htons((uint16_t) port_number);
    sa.sin_addr.s_addr = inet_addr(ip);

    init_connection init;
    memset(&init, 0, sizeof(init_connection));
    strncpy(init.uuid, uuid, UUID_LENGTH);
    init.downloading = false;

    if (!open_connection(init)) {
        fprintf(stderr, "Can't connect to %s:%d, Exiting...", address, port_number);
        exit(1);
    }

    bool running = true;
    while (running) {
        switch(state) {
            case CLIENT_IDLE : {
                running = process_user_input(&download_info);
                break;
            }
            case CLIENT_DOWNLOADING: {
                download_file_part(&download_info);
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

bool open_connection(init_connection init) {
    fd_skt = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(fd_skt, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
        return false;
    }

    set_socket_timeout();
    send_data(fd_skt, &init, sizeof(init_connection), 0);
    return true;
}

void set_socket_timeout() {
    struct timeval tv;
    tv.tv_sec = CONN_TIMEOUT;
    tv.tv_usec = 0;  // Not init'ing this can cause strange errors
    setsockopt(fd_skt, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
}

void download_file_part(file_info* download_info) {
    static int parts = 1;
    static ssize_t nread;
    static char buf[CHUNK_SIZE];

    size_t size = download_info->size;

    nread = recv(fd_skt, buf, CHUNK_SIZE, 0);
    if (nread <= 0) {
        reopen_socket();
        return;
    } else {
        fwrite(buf, sizeof(char), (size_t) nread, file);
    }

    download_info->offset += nread;
    off_t offset = download_info->offset;
    if (offset > PART_SIZE * parts) {
        parts++;
        print_speed_info(offset, size);
    }

    if (offset >= size) {
        printf("Download finished!!\n");
        fclose(file);
        state = CLIENT_IDLE;
    }
}

void reopen_socket() {
    printf("Problem with connection... Cannot download file\n");
    close(fd_skt);

    char answer = 0;
    printf("Try to restore connection? Y/N\n");
    while (answer != 'Y' && answer != 'N') {
        scanf("%c", &answer);
    }

    if (answer == 'N') {
        exit(0);
    }

    init_connection init;
    memset(&init, 0, sizeof(init_connection));
    strncpy(init.uuid, uuid, UUID_LENGTH);
    if (state == CLIENT_DOWNLOADING) {
        init.downloading = true;
        init.offset = download_info.offset;
    } else {
        init.downloading = false;
    }

    if (!open_connection(init)) {
        printf("Cannot open new connection. Exiting...\n");
        exit(1);
    }
    printf("Success! Connection reopened!");
    state = CLIENT_IDLE;
}

void print_speed_info(off_t downloaded, size_t total) {
    static struct timeval t2;
    gettimeofday(&t2, NULL);
    double elapsed = (double) (t2.tv_usec - t1.tv_usec) / 1000000 +
              (double) (t2.tv_sec - t1.tv_sec);

    char speed_str[12];
    char finished_str[12];
    memset(speed_str, 0, sizeof(speed_str));
    memset(finished_str, 0, sizeof(finished_str));

    get_size_str(downloaded / elapsed, speed_str);
    get_size_str(downloaded, finished_str);

    printf("%s downloaded (%.1f%%). Elapsed time is %.3f sec. Average speed is %sps \n",
           finished_str,
           (double) downloaded * 100 / total,
           elapsed,
           speed_str);
}

void init_download(char *file_path, file_info* download_info) {
    static ssize_t nread;
    char* file_name = basename(file_path);
    file_name[strlen(file_name) - 1] = '\0';

    memset(download_info, 0, sizeof(download_info));

    nread = recv(fd_skt, download_info, sizeof(file_info), 0);
    if (nread != sizeof(file_info)) {
        printf("Problem occurred. Cannot download file\n");
        state = CLIENT_IDLE;
    }

    if (strcmp(file_name, download_info->name)) {
        state = CLIENT_IDLE;
        return;
    }

    if (strlen(download_info->comment) > 0) {
        printf("%s\n", download_info->comment);
    }

    if (download_info->size == 0) {
        state = CLIENT_IDLE;
        return;
    }

    file = fopen(download_info->name, "a");
    state = CLIENT_DOWNLOADING;
    gettimeofday(&t1, NULL);
}

bool process_user_input(file_info* download_info) {
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
        reopen_socket();
        return true;
    }

    if (response.success && strlen(response.text) > 0) {
        printf("%s\n", response.text);
    }

    if (starts_with(input_buf, COMMAND_DOWNLOAD)
        && response.success
        && !command_found) {

        init_download(input_buf, download_info);
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
        reopen_socket();
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