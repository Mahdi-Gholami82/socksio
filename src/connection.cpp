#include <asio.hpp>
#include <asio/read.hpp>
#include <asio/use_future.hpp>
#include <asio/write.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <print>
#include "connection.hpp"

using asio::ip::tcp;

connection::pointer connection::create(asio::io_context &io_context, tcp::endpoint endpoint) {
    return pointer(new connection(io_context, endpoint));
}

tcp::socket& connection::socket() {
    return _socket;
}

connection::connection(asio::io_context& io_context, tcp::endpoint endpoint) : _socket(io_context) {};
