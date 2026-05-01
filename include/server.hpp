#pragma once

#include <asio.hpp>
#include <asio/awaitable.hpp>

#include "connection.hpp"
#include "socks.hpp"

class server {
    public:
        server(asio::io_context& io_context, tcp::endpoint endpoint);
        asio::io_context& io_context;
        tcp::endpoint endpoint;
        asio::awaitable<void> listen();
        asio::awaitable<void> serve(std::shared_ptr<socks::socks_connection> socks_connection);
        void async_start();
        tcp::acceptor& acceptor();

    private:
        tcp::acceptor acceptor_;
};
