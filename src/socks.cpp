#include <array>
#include <asio.hpp>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include <print>

#include "socks_spec.hpp"
#include "socks.hpp"

using namespace socks_spec;

namespace socks {

    // socks_connection
    asio::awaitable<handshake_result> socks_connection::do_initial_handshake() {
        std::array initial_buffer = std::array<std::uint8_t,2>();
        co_await read_bytes(asio::buffer(initial_buffer));
        const socks_spec::version socks_version = get_version(initial_buffer[0]);
        const uint8_t number_of_methods = initial_buffer[1];
        std::vector<std::uint8_t> methods_buffer(number_of_methods);
        co_await read_bytes(asio::buffer(methods_buffer));
        co_return handshake_result(socks_version,std::move(methods_buffer));
    }

    asio::awaitable<void> socks_connection::write_method_selection(method method) {
        std::array<uint8_t, 2> message_buffer = {to_underlying(version::VERSION5),to_underlying(method)};
        co_await write_bytes(asio::buffer(message_buffer));
    }

    asio::awaitable<request> socks_connection::read_request() {
        std::array initial_buffer = std::array<uint8_t,4>();
        co_await read_bytes(asio::buffer(initial_buffer,sizeof(initial_buffer)));
        if (initial_buffer[2] != 0x00) {
            throw std::runtime_error("Reserved byte must be 0x00");
        }
        version version = get_version(initial_buffer[0]);
        command command = get_command(initial_buffer[1]);
        std::string destination_address;
        switch (get_address_type(initial_buffer[3])) {
            case socks_spec::address_type::IPV4: {
                asio::ip::address_v4::bytes_type ipv4_buffer;
                co_await read_bytes(asio::buffer(ipv4_buffer,sizeof(ipv4_buffer)));
                asio::ip::address_v4 address = asio::ip::make_address_v4(ipv4_buffer);
                destination_address = address.to_string();
                break;
            }
            case socks_spec::address_type::IPV6: {
                asio::ip::address_v6::bytes_type ipv6_buffer;
                co_await read_bytes(asio::buffer(ipv6_buffer,sizeof(ipv6_buffer)));
                asio::ip::address_v6 address = asio::ip::make_address_v6(ipv6_buffer);
                destination_address = address.to_string();
                break;
            }
            case socks_spec::address_type::DOMAIN: {
                uint8_t domain_lenght;
                co_await read_bytes(asio::buffer(&domain_lenght,sizeof(domain_lenght)));
                std::string domain_buffer;
                domain_buffer.resize(domain_lenght);
                co_await read_bytes(asio::buffer(domain_buffer,domain_lenght));
                destination_address = domain_buffer;
                break;
            }
            default: {
                throw std::runtime_error("Bad address type");
                break;
            }
        }
        asio::ip::port_type port_buffer;
        co_await read_bytes(asio::buffer(&port_buffer,sizeof(port_buffer)));
        co_return request(
            version,
            command,
            get_address_type(initial_buffer[3]),
            destination_address,
            asio::detail::socket_ops::network_to_host_short(port_buffer));
    }

    // reply
    std::vector<uint8_t> reply::to_buffer() {
        std::vector<uint8_t> buf = {
            to_underlying(version),
            to_underlying(reply_code),
            socks_spec::ZERO,
            to_underlying(address_type)
        };
        
        if (!bound_address.empty()) {
            if (address_type == socks_spec::address_type::DOMAIN) {
                buf.push_back(static_cast<uint8_t>(bound_address.size()));
                buf.insert(buf.end(), bound_address.begin(), bound_address.end());
            } else {
                asio::ip::address address = asio::ip::make_address(bound_address);
                if (address.is_v4()) {
                    auto ipv4_bytes = address.to_v4().to_bytes();
                    buf.insert(buf.end(), ipv4_bytes.begin(), ipv4_bytes.end());
                } else if (address.is_v6()) {
                    auto ipv6_bytes = address.to_v6().to_bytes();
                    buf.insert(buf.end(), ipv6_bytes.begin(), ipv6_bytes.end());
                } else {
                    throw std::runtime_error("IP type not specified");
                }
            }
        } else {
            if (address_type == socks_spec::address_type::IPV4) {
                buf.insert(buf.end(), 4, 0x00);
            } else if (address_type == socks_spec::address_type::IPV6) {
                buf.insert(buf.end(), 16, 0x00);
            } else if (address_type == socks_spec::address_type::DOMAIN) {
                buf.push_back(0x00);
            }
        }
        
        // Convert port to network byte order using asio
        asio::ip::port_type port_network = asio::detail::socket_ops::host_to_network_short(bound_port);
        buf.push_back(static_cast<uint8_t>((port_network >> 8) & 0xFF));
        buf.push_back(static_cast<uint8_t>(port_network & 0xFF));
        
        return buf;
    }

    // request
    std::string request::to_string()  {
        std::string text = "";
        switch (command) {
            case socks_spec::command::CONNECT: {
                text = "CONNECT";
                break;
            }
            case socks_spec::command::UDP_ASSOCIATE: {
                text = "UDP_ASSOCIATE";
                break;
            }
            case socks_spec::command::BIND: {
                text = "BIND";
                break;
            }
            default:
                throw std::logic_error("Invalid command");
        }
        std::string address = destination_address;
        if (address_type == socks_spec::address_type::IPV6) {
            address = '[' + address + ']';
            std::println("{}",destination_address);
        }
        text += std::format(" {}:{}",address,destination_port);
        return text;

    }
}