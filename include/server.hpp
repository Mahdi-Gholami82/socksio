#pragma once

#include <asio.hpp>

#include "connection.hpp"
#include "socks.hpp"

class server {
    public:
        server(asio::io_context& io_context, tcp::endpoint endpoint);
        asio::io_context& io_context;
        tcp::endpoint endpoint;
        asio::coroutine _coroutine;
        asio::awaitable<void> async_start(asio::use_awaitable_t<>);
        void async_start();
        tcp::acceptor& acceptor();

    private:
        tcp::acceptor _acceptor;
};
