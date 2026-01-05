#include "proxy/socket_utils.hpp"

#include <stdexcept>

namespace proxy {

bool init_network() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
#endif
    return true;
}

void cleanup_network() {
#ifdef _WIN32
    WSACleanup();
#endif
}

void close_socket(socket_t fd) {
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

socket_t create_socket() {
    socket_t fd = socket(AF_INET, SOCK_STREAM, 0);
    
#ifdef _WIN32
    if (fd == INVALID_SOCKET) {
        throw std::runtime_error("Erro ao criar socket");
    }
#else
    if (fd < 0) {
        throw std::runtime_error("Erro ao criar socket");
    }
#endif
    
    return fd;
}

}