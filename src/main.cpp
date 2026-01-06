#include <iostream>
#include <string>

#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>

#include "proxy/platform.hpp"
#include "proxy/socket_utils.hpp"
#include "proxy/http_parser.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: ./proxy <porta>\n";
        return 1;
    }

    if (!proxy::init_network()) {
        std::cerr << "Falha ao inicializar rede\n";
        return 1;
    }

    spdlog::set_level(spdlog::level::info);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");

    socket_t server_fd = proxy::create_socket();

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(std::stoi(argv[1]));

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        spdlog::error("Falha no bind");
        proxy::cleanup_network();
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        spdlog::error("Falha no listen");
        proxy::cleanup_network();
        return 1;
    }

    spdlog::info("Proxy rodando na porta {}", argv[1]);

    while (true) {
        socket_t client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd == INVALID_SOCKET_VALUE) continue;

        std::string request = proxy::read_full_request(client_fd);
        if (request.empty()) {
            proxy::close_socket(client_fd);
            continue;
        }

        std::string host = proxy::extract_host(request);

        // Log JSON
        nlohmann::json log_entry = {
            {"event", "http_request"},
            {"client_fd", static_cast<int>(client_fd)},
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
            proxy::close_socket(client_fd);
            continue;
        }

        socket_t remote_fd = proxy::create_socket();
        if (connect(remote_fd, res->ai_addr, static_cast<int>(res->ai_addrlen)) < 0) {
            spdlog::error("Falha ao conectar no host {}", host);
            freeaddrinfo(res);
            proxy::close_socket(client_fd);
            proxy::close_socket(remote_fd);
            continue;
        }

        freeaddrinfo(res);

        send(remote_fd, request.c_str(), static_cast<int>(request.size()), 0);

        char buffer[proxy::BUFFER_SIZE];
        int bytes;
        while ((bytes = recv(remote_fd, buffer, proxy::BUFFER_SIZE, 0)) > 0) {
            send(client_fd, buffer, bytes, 0);
        }

        proxy::close_socket(remote_fd);
        proxy::close_socket(client_fd);
    }

    proxy::close_socket(server_fd);
    proxy::cleanup_network();

    return 0;
}
