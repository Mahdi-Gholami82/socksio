#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "connection.hpp"

using asio::ip::tcp;

class socks_connection {
    public:
        uint8_t version;
        std::array<uint8_t, 255> supported_methods;

        static asio::awaitable<socks_connection> initiate(std::shared_ptr<connection> connection);

};