#ifndef PROXY_HTTP_PARSER_HPP
#define PROXY_HTTP_PARSER_HPP

#include <string>
#include "platform.hpp"

namespace proxy {

std::string extract_host(const std::string& request);

size_t get_content_length(const std::string& request);

std::string read_full_request(socket_t client_fd);

}

#endif