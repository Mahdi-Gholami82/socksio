#pragma once

#include <asio.hpp>
#include <memory>

using asio::ip::tcp;

class connection {
    public:
        using pointer = typename std::shared_ptr<connection>;
        
        static pointer create(asio::io_context& io_context, tcp::endpoint endpoint);
        
        tcp::socket& socket();
        

    private:
        connection(asio::io_context& io_context, tcp::endpoint endpoint);

        tcp::socket _socket;
};