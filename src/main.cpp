#include <asio.hpp>
#include <asio/ip/address.hpp>
#include <optional>
#include <print>
#include <string>

#include "arguments_parser.hpp"
#include "server.hpp"
#include "logger.hpp"

using asio::ip::tcp;

void print_help(argp::argument_list& argument_list) {
    std::println("Usage: socksio [options]\nOptions:\n{}",
                argument_list.generate_help());
}

int main(int argc, char* argv[]) {
    argp::argument_list argument_list(argc, argv);
    bool asked_for_help = argument_list.is_specified(
        argp::argument("help", "Show this help message and exit"));
    std::optional<std::string> listen_address_arg = argument_list.get(
        argp::argument("addr", "Listen address, Default: 127.0.0.1", 'a'));
    std::optional<int> listen_port_arg = argument_list.get_num(
        argp::argument("port", "Listen port number, Default: 1080"));
    if (asked_for_help) {
        print_help(argument_list);
        return 0;
    }
    try {
        argument_list.validate();
    } catch (argp::argument_validation_error& error) {
        std::println("invalid arguments :");
        for (const std::string& element : error.invalid_arguments) {
            std::print("{} ",element);
        }
        std::print("\n\n");
        print_help(argument_list);
        return 1;
    }
    asio::ip::address listen_address =
        listen_address_arg.has_value()
            ? asio::ip::make_address(listen_address_arg.value())
            : asio::ip::address_v4::loopback();
    logger.set_log_level(log_level::info);
    asio::io_context io_context;
    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, int signal){ 
        logger.warn("SIGNAL {} received",signal);
        io_context.stop(); 
    });
    tcp::endpoint endpoint(listen_address, listen_port_arg.has_value() ? *listen_port_arg : 1080);
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

