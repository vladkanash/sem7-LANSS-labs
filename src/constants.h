#define COMMAND_ECHO "ECHO "
#define COMMAND_TIME "TIME"
#define COMMAND_CLOSE "CLOSE"
#define COMMAND_DOWNLOAD "DOWNLOAD "
#define COMMAND_UPLOAD "UPLOAD "

#define BUF_SIZE 256
#define COMMAND_MAX_LENGTH sizeof(COMMAND_DOWNLOAD)
#define COMMAND_COUNT 4
#define MAX_CLIENTS 256

#define COMMAND_END_1 "\n"
#define COMMAND_END_2 "\r\n"
#define ARGS_DELIMITER " "

#define CHUNK_SIZE 512

#define URANDOM_PATH "/dev/urandom"
#define UUID_LENGTH 16