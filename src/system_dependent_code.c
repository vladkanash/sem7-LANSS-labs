#include "system_dependent_code.h"

int get_process_id(){
  #ifdef _WIN32
    return _getpid();
  #else
    return getpid();
  #endif
}

int winsock_version(){
	 return (int)0x0202; // equal to windows macros MAKEWORD(2,2);
}

int initialize_windows_socket(int (*version)()){
	WSADATA wsa;
	int fd_skt;

    if (WSAStartup(version(), &wsa) != 0) {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }

    if((fd_skt = socket(AF_INET, SOCK_STREAM, 0 )) == INVALID_SOCKET) {
        printf("Could not create socket : %d" , WSAGetLastError());
    }

	return fd_skt;
}

int initialize_unix_socket(){
	return (int)socket(AF_INET, SOCK_STREAM, 0);
}



int initialize_socket(){
	#ifdef _WIN32
		return initialize_windows_socket(winsock_version);
	#else
		return initialize_unix_socket
	#endif
}

int quit_socket(){
  #ifdef _WIN32
    return WSACleanup();
  #else
    return 0;
  #endif
}

int close_socket(int socket){
  int status = 0;

  #ifdef _WIN32
    status = shutdown(socket, SD_BOTH);
    if (status == 0) { status = closesocket(socket); }
  #else
    status = shutdown(socket, SHUT_RDWR);
    if (status == 0) { status = close(socket); }
  #endif

  return status;
}

int send_data(int soket, const char *buffer, int length, int flags){
    #ifdef _WIN32
        return send(soket, buffer, length, flags);
    #else
        return write(soket, buffer, length);
    #endif
}
