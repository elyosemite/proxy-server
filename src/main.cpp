#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
// Logging, cache, security, performance, etc.
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socklen_t = int;
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <sys/socket.h>
#endif

constexpr int BUFFER_SIZE = 8 * 1024; // 8KB

void close_socket(int fd) {
#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

std::string extract_host(const std::string& request) {
    auto pos = request.find("Host:");
    if (pos == std::string::npos) return "";

    pos += 5;
    while (pos < request.size() && request[pos] == ' ') pos++;

    auto end = request.find("\r\n", pos);
    return request.substr(pos, end - pos);
}

std::string read_full_request(int client_fd) {
    std::string request;
    char buffer[BUFFER_SIZE];

    while (true) {
        int bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) break;

        request.append(buffer, bytes);

        if (request.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    auto header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) return request;

    size_t body_start = header_end + 4;
    size_t content_length = 0;

    // Extrair Content-Length se presente
    auto cl_pos = request.find("Content-Length:");
    if (cl_pos != std::string::npos && cl_pos < header_end) {
        cl_pos += 15;
        while (cl_pos < request.size() && request[cl_pos] == ' ') cl_pos++;
        content_length = std::stoul(request.substr(cl_pos));
    }

    // Ler o restante do corpo se necessário
    size_t body_received = request.size() - body_start;
    while (body_received < content_length) {
        int bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) break;

        request.append(buffer, bytes);
        body_received += bytes;
    }

    return request;
}

int create_socket() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) throw std::runtime_error("Erro ao criar socket");
    return fd;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: ./proxy <porta>\n";
        return 1;
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "Falha ao inicializar Winsock\n";
        return 1;
    }
#endif

    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    int server_fd = create_socket();

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(std::stoi(argv[1]));

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        spdlog::error("Falha no bind");
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        spdlog::error("Falha no listen");
        return 1;
    }

    spdlog::info("Proxy rodando na porta {}", argv[1]);

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) continue;

        char buffer[BUFFER_SIZE]{};
        int recv_bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        if (recv_bytes <= 0) {
            close_socket(client_fd);
            continue;
        }

        std::string request(buffer, recv_bytes);
        std::string host = extract_host(request);

        // Log JSON
        nlohmann::json log_entry = {
            {"event", "http_request"},
            {"client_fd", client_fd},
            {"host", host},
            {"request_size", request.size()}
        };
        spdlog::info("{}", log_entry.dump());

        struct addrinfo hints{}, *res = nullptr;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int status = getaddrinfo(host.c_str(), "80", &hints, &res);
        if (status != 0 || res == nullptr) {
            spdlog::error("Falha em getaddrinfo para host {}: {}", host, gai_strerror(status));
            close_socket(client_fd);
            continue;
        }

        int remote_fd = create_socket();
        if (connect(remote_fd, res->ai_addr, res->ai_addrlen) < 0) {
            spdlog::error("Falha ao conectar no host {}", host);
            freeaddrinfo(res);
            close_socket(client_fd);
            close_socket(remote_fd);
            continue;
        }

        freeaddrinfo(res);

        send(remote_fd, request.c_str(), request.size(), 0);

        int bytes;
        while ((bytes = recv(remote_fd, buffer, BUFFER_SIZE, 0)) > 0) {
            send(client_fd, buffer, bytes, 0);
        }

        close_socket(remote_fd);
        close_socket(client_fd);
    }

    close_socket(server_fd);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
