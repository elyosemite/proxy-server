#include "proxy/http_parser.hpp"

namespace proxy {

std::string extract_host(const std::string& request) {
    auto pos = request.find("Host:");
    if (pos == std::string::npos) {
        return "";
    }

    pos += 5;
    while (pos < request.size() && request[pos] == ' ') {
        pos++;
    }

    auto end = request.find("\r\n", pos);
    if (end == std::string::npos) {
        return request.substr(pos);
    }

    return request.substr(pos, end - pos);
}

size_t get_content_length(const std::string& headers) {
    auto pos = headers.find("Content-Length:");
    if (pos == std::string::npos) {
        pos = headers.find("content-length:");
    }
    if (pos == std::string::npos) {
        return 0;
    }

    pos += 15; // Length of "Content-Length:"
    while (pos < headers.size() && headers[pos] == ' ') {
        pos++;
    }

    try {
        return std::stoul(headers.substr(pos));
    } catch (...) {
        return 0;
    }
}

std::string read_full_request(socket_t client_fd) {
    std::string request;
    request.reserve(BUFFER_SIZE);

    char buffer[BUFFER_SIZE];

    while (true) {
        int bytes = recv(client_fd, buffer, static_cast<int>(BUFFER_SIZE), 0);

        if (bytes <= 0) {
            break;
        }

        request.append(buffer, bytes);

        if (request.find("\r\n\r\n") != std::string::npos) {
            break;
        }
    }

    auto header_end = request.find("\r\n\r\n");
    if (header_end == std::string::npos) {
        return request;
    }

    std::string headers = request.substr(0, header_end);
    size_t content_length = get_content_length(headers);

    if (content_length == 0) {
        return request;
    }

    size_t body_start = header_end + 4;
    size_t body_received = request.size() - body_start;
    
    while (body_received < content_length)
    {
        int bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
        
        if (bytes <= 0) {
            break;
        }

        request.append(buffer, bytes);
        body_received += bytes;
    }
    return request;
}

}