#pragma once

#include <asio.hpp>
#include <asio/awaitable.hpp>
#include <asio/write.hpp>
#include <memory>

using asio::ip::tcp;

class tcp_connection {
    using pointer = typename std::shared_ptr<tcp_connection>;
    public:
        tcp_connection(asio::io_context& io_context, tcp::endpoint endpoint);
        
        static pointer create(asio::io_context& io_context, tcp::endpoint endpoint);
        
        tcp::socket& socket();

        template <typename MutableBufferSequence>
        requires asio::is_mutable_buffer_sequence<MutableBufferSequence>::value
        inline asio::awaitable<void> read_bytes(MutableBufferSequence buffer) {
            co_await asio::async_read(_socket,buffer,asio::use_awaitable);
        }

        template <typename ConstBufferSequence>
        requires asio::is_const_buffer_sequence<ConstBufferSequence>::value
        inline asio::awaitable<void> write_bytes(ConstBufferSequence buffer) {
            co_await asio::async_write(_socket,buffer,asio::use_awaitable);
        }
        

    private:

        tcp::socket _socket;
};