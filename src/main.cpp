#include <asio.hpp>
#include <cstdio>
#include <print>

#include "server.hpp"

using asio::ip::tcp;

int main(int argc, char* argv[]) {
    
    asio::io_context io_context;
    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context.stop(); });
    tcp::endpoint endpoint(asio::ip::make_address("127.0.0.1"), 2522);
    server server(io_context,endpoint);
    server.async_start();
    io_context.run();
    return 0;
}

