#include <asio.hpp>
#include <cstdio>
#include <print>

#include "connection.hpp"
#include "socks.hpp"
#include "server.hpp"

server::server(asio::io_context& io_context, tcp::endpoint endpoint): io_context(io_context), _acceptor(io_context, endpoint){}

tcp::acceptor& server::acceptor() {
    return _acceptor;
}

asio::awaitable<void> server::async_start(asio::use_awaitable_t<>) {
    while(true) {
        connection::pointer connection_ptr = connection::create(io_context, endpoint);

        co_await acceptor().async_accept(
            connection_ptr->socket(),
            asio::use_awaitable);
        tcp::endpoint remote_endpoint = connection_ptr->socket().remote_endpoint();
        std::println("Accepted connection {}",remote_endpoint.address().to_string());
        co_await socks_connection::initiate(connection_ptr);
    }
}

void server::async_start() {
    asio::co_spawn(io_context,async_start(asio::use_awaitable),asio::detached);
}
