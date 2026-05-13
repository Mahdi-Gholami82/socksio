#pragma once

#include <asio.hpp>
#include <asio/write.hpp>
#include <memory>

using asio::ip::tcp;

template<typename T>
concept is_mutable_buffer_sequence_v = asio::is_mutable_buffer_sequence<T>::value;

template<typename T>
concept is_const_buffer_sequence_v = asio::is_const_buffer_sequence<T>::value;

template<typename T>
concept is_buffer_sequence_v = asio::is_mutable_buffer_sequence<T>::value || asio::is_dynamic_buffer<T>::value;

class tcp_connection {
    using pointer = typename std::shared_ptr<tcp_connection>;
    public:
        tcp_connection(asio::io_context& io_context, tcp::endpoint endpoint);
        
        static pointer create(asio::io_context& io_context, tcp::endpoint endpoint);
        
        tcp::socket& socket();

        template <typename BufferSequence>
        requires is_buffer_sequence_v<BufferSequence>
        inline asio::awaitable<void> read_bytes(BufferSequence buffer) {
            co_await asio::async_read(_socket,buffer,asio::use_awaitable);
        }

        template <typename ConstBufferSequence>
        requires is_const_buffer_sequence_v<ConstBufferSequence>
        inline asio::awaitable<void> write_bytes(const ConstBufferSequence buffer) {
            co_await asio::async_write(_socket,buffer,asio::use_awaitable);
        }
        

    private:

        tcp::socket _socket;
};