#include <asio.hpp>
#include <cstdio>
#include "connection.hpp"

using asio::ip::tcp;

tcp_connection::pointer tcp_connection::create(asio::io_context &io_context, tcp::endpoint endpoint) {
    return pointer(new tcp_connection(io_context, endpoint));
}

tcp::socket& tcp_connection::socket() {
    return _socket;
}

tcp_connection::tcp_connection(asio::io_context& io_context, tcp::endpoint endpoint) : _socket(io_context) {};
