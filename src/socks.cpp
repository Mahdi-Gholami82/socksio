#include <array>
#include <cstdint>
#include <print>
#include <asio.hpp>

#include "connection.hpp"
#include "socks_spec.hpp"
#include "socks.hpp"

using std::println;

asio::awaitable<socks_connection> socks_connection::initiate(std::shared_ptr<connection> connection) {
    std::array initial_buffer = std::array<std::uint8_t,2>();
    socks_connection socks_connection;
    co_await asio::async_read(
        connection->socket(),
        asio::buffer(initial_buffer,sizeof(initial_buffer)),
        asio::use_awaitable);
    if (initial_buffer[0] == socks_spec::version::VERSION5) {
        socks_connection.version = initial_buffer[0];
        const uint8_t number_of_methods = initial_buffer[1];
        std::array methods_buffer = std::array<std::uint8_t,socks_spec::method::methods_list.size()>();
        co_await asio::async_read(connection->socket(),asio::buffer(methods_buffer,number_of_methods * sizeof(uint8_t)),asio::use_awaitable);
        for (int index = 0;index < number_of_methods;index++) {
            println("{}",methods_buffer[index]);
        }
        co_await asio::async_write(
            connection->socket(),
            asio::buffer({socks_spec::version::VERSION5,socks_spec::method::NO_ACCEPTABLE_METHODS}),
            asio::use_awaitable
        );
        println("wrote finish.");
        co_return socks_connection; 
    } else {
        throw std::runtime_error("No supported socks version.");
    }
}