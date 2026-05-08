#pragma once

#include <asio.hpp>
#include <asio/awaitable.hpp>
#include <cstdint>
#include <memory>
#include <vector>

#include "connection.hpp"
#include "socks_spec.hpp"

using asio::ip::tcp;

namespace socks {
    constexpr uint16_t RELAY_BUFFER_SIZE = 32768;

    struct handshake_result;
    struct request;

    asio::awaitable<request> read_request(std::shared_ptr<tcp_connection> connection);
    asio::awaitable<handshake_result> do_initial_handshake(std::shared_ptr<tcp_connection> connection);

    class socks_connection : public tcp_connection {
        public:
            socks_connection(asio::io_context& io_context, tcp::endpoint endpoint) : tcp_connection(io_context,endpoint) {}
            asio::awaitable<handshake_result> do_initial_handshake();
            asio::awaitable<void> write_method_selection(socks_spec::method method);
            asio::awaitable<request> read_request();
    };

    struct handshake_result {
        const socks_spec::version version;
        const std::vector<uint8_t> supported_methods;

        handshake_result(socks_spec::version version,std::vector<uint8_t> supported_methods) :
            version(version), 
            supported_methods(std::move(supported_methods)) {}

        const bool is_method_supported(socks_spec::method method)  {
            return std::find_if(supported_methods.begin(),supported_methods.end(),[&method](uint8_t v){
                return v == method;
            }) != supported_methods.end();
        }
    };

    class request {
        public:
          request(socks_spec::version version, socks_spec::command command,
                  socks_spec::address_type address_type,
                  std::string destination_address,
                  asio::ip::port_type destination_port)
              : version(version), command(command), address_type(address_type),
                destination_address(std::move(destination_address)),
                destination_port(destination_port) {}

          const socks_spec::version version;
          const socks_spec::command command;
          const socks_spec::address_type address_type;
          const std::string destination_address;
          const asio::ip::port_type destination_port;

          std::string to_string();
            };
            
    class reply {
        public:
        const socks_spec::version version;
        const socks_spec::reply_code reply_code;
        const socks_spec::address_type address_type;
        const std::string bound_address;
        const asio::ip::port_type bound_port;
        std::vector<uint8_t> buffer;
        
        reply(socks_spec::version version,socks_spec::reply_code reply_code,socks_spec::address_type address_type,std::string bound_address,uint8_t bound_port) 
        : version(version),reply_code(reply_code),address_type(address_type),bound_address(bound_address),bound_port(bound_port) {
            buffer = to_buffer();
        }

        inline static reply empty(socks_spec::version version,socks_spec::reply_code reply_code) {
            return reply(version,reply_code,socks_spec::address_type::EMPTY,"",socks_spec::ZERO);
        }

        private:
        std::vector<uint8_t> to_buffer();
    };

    class connection {
    public:
        const uint8_t version;
        std::vector<uint8_t> supported_methods;
    };
}

