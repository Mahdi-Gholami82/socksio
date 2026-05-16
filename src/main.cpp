#include <algorithm>
#include <asio.hpp>
#include <asio/ip/address.hpp>
#include <optional>
#include <string>
#include <utility>

#include "argument_parser.hpp"
#include "server.hpp"
#include "logger.hpp"

using asio::ip::tcp;

log_level log_level_from(argp::result<std::string> text_result) {
    auto it = std::find_if(log_names.begin(),log_names.end(),[&text = text_result.value] (const std::pair<log_level, std::string>& pair) {
        if (text == pair.second.substr(0,text.length())) {
            return true;
        }
        return false;
    });
    if (it != log_names.end()) {
        return it->first;
    }
    throw argp::invalid_argument_value("Invalid log level text",text_result.captured_from,text_result.value);
}

class args : public argp::args_template {
    public:
        args(int argc, char* argv[]) : argp::args_template(argc, argv) {}
        argp::result<std::string> log_level = "info";
        argp::result<std::string> listen_address = "0.0.0.0";
        argp::result<int> listen_port = 1080;
        argp::result<bool> show_help = false;

        std::string get_help() {
            return std::format("Usage: socksio [options]\nOptions:\n{}",
                manager_.generate_help());
        }
    private:
        void initiate() override {
            define_arg(log_level,argp::argument(
                    "log-level", "Logging level, Default: info (debug|info|warn|error|critical)"));
            define_arg(listen_address,argp::argument(
                    "addr", "Listen address, Default: 0.0.0.0", 'a'));
            define_arg(listen_port,argp::argument(
                    "port", "Listen port number, Default: 1080"));
            define_arg(show_help,argp::argument(
                    "help", "Show this help message and exit"));
        }
};

int main(int argc, char* argv[]) {
    args args(argc,argv);
    try {
        args.extract();
        if (args.show_help.value) {
            logger.raw("{}",args.get_help());
            return 0;
        }
        log_level log_level;
        log_level = log_level_from(args.log_level);
        logger.set_log_level(log_level);
        asio::ip::address listen_address;
        try {
            listen_address = asio::ip::make_address(args.listen_address.value);
        } catch (const std::system_error&) {
            throw argp::invalid_argument_value("Invalid listen address format",args.listen_address.captured_from,args.listen_address.value);
        }
        asio::io_context io_context;
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, int signal){ 
            logger.warn("SIGNAL {} received",signal);
            io_context.stop(); 
        });
        tcp::endpoint endpoint(listen_address, args.listen_port.value);
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
    } catch (argp::invalid_argument_value& error) {
        
        logger.raw_err("invalid value for {} \n{}",error.target_argument,args.get_help());
        return 1;
    } catch (argp::argument_validation_error& error) {
        std::string error_text = "invalid arguments :\n";
        for (const std::string& element : error.invalid_arguments) {
            error_text += element + " ";
        }
        error_text += std::format("\n\n{}",args.get_help());
        logger.raw_err("{}", error_text);
        return 1;
    }
    return 0;
}

