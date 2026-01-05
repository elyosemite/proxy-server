#ifndef PROXY_SOCKET_UTILS_HPP
#define PROXY_SOCKET_UTILS_HPP

#include "platform.hpp"

namespace proxy {
bool init_network();

void cleanup_network();

void close_socket(socket_t fd);

socket_t create_socket();
}

#endif