#ifndef PROXY_PLATFORM_HPP
#define PROXY_PLATFORM_HPP

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socklen_t = int;
    using socket_t = SOCKET;
    constexpr socket_t INVALID_SOCKET_VALUE = INVALID_SOCKET;
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/socket.h>
    using socket_t = int;
    constexpr socket_t INVALID_SOCKET_VALUE = -1;
#endif

namespace proxy {
    constexpr int BUFFER_SIZE = 8 * 1024; // 8KB
}

#endif