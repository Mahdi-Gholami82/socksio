#include <asio.hpp>
#include <print>

#include "server.hpp"
#include "logger.hpp"

using asio::ip::tcp;

int main(int argc, char* argv[]) {
    
    asio::io_context io_context;
    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, int signal){ 
        logger.warn("SIGNAL {} received",signal);
        io_context.stop(); 
    });
    tcp::endpoint endpoint(asio::ip::address_v4::loopback(), 2522);
    server server(io_context,endpoint);
    try {
        server.async_start();
    } catch (std::system_error& error) {
        if (error.code().value() == asio::error::address_in_use) {
            logger.critical("Address already in use");
        } else {
            logger.critical("{}", error.what());
        }
        return 1;
    }
    io_context.run();
    return 0;
}

