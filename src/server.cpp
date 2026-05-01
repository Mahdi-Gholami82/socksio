#include <asio.hpp>
#include <asio/error.hpp>
#include <asio/ip/address.hpp>
#include <asio/ip/tcp.hpp>
#include <format>
#include <memory>
#include <print>
#include <stdexcept>
#include <string>
#include <system_error>

#include "connection.hpp"
#include "logger.hpp"
#include "socks.hpp"
#include "socks_spec.hpp"
#include "server.hpp"

using namespace socks_spec;

server::server(asio::io_context& io_context, tcp::endpoint endpoint): io_context(io_context), endpoint(endpoint), acceptor_(io_context){}

tcp::acceptor& server::acceptor() {
    return acceptor_;
}

void server::async_start() {
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    asio::co_spawn(io_context,listen(),asio::detached);
}

asio::awaitable<void> server::listen() {
    while(true) {
        std::shared_ptr connection_ptr = std::make_shared<socks::socks_connection>(io_context, endpoint);
        co_await acceptor().async_accept(
            connection_ptr->socket(),
            asio::use_awaitable);
        asio::co_spawn(
            io_context,
            [connection_ptr = std::move(connection_ptr),this]() -> asio::awaitable<void> {
                co_await serve( std::move(connection_ptr));
            },
            asio::detached);
    }
}
        
asio::awaitable<void> server::serve(std::shared_ptr<socks::socks_connection> connection_ptr) {
        tcp::endpoint remote_endpoint = connection_ptr->socket().remote_endpoint();
        socks::handshake_result result = co_await connection_ptr->do_initial_handshake();
        auto prefered_method = method::NO_AUTHENTICATION_REQUIRED;
        auto set_version = version::VERSION4;
        if (!result.is_method_supported(prefered_method) ) {
            co_await connection_ptr->write_method_selection(socks_spec::method::NO_ACCEPTABLE_METHODS);
            co_return;
        }
        co_await connection_ptr->write_method_selection(prefered_method);
        socks::request request = co_await connection_ptr->read_request();
        logger.info("Request: {} from {}",request.to_string(),remote_endpoint.address().to_string());
        if (request.command != socks_spec::command::CONNECT) {
            co_await connection_ptr->write_bytes(
                asio::buffer(socks::reply::empty(result.version,socks_spec::reply_code::COMMAND_NOT_SUPPORTED).buffer)
            );
            co_return;
        }
        tcp::endpoint endpoint;
        if (request.address_type == socks_spec::address_type::DOMAIN) {
            tcp::resolver resolver(io_context);
            tcp::resolver::results_type endpoints = co_await resolver.async_resolve(
                request.destination_address,
                std::to_string(request.destination_port),
                asio::use_awaitable
            );
        endpoint = endpoints->endpoint();
        } else {
            endpoint = tcp::endpoint(asio::ip::make_address(request.destination_address),request.destination_port);
        }
        std::shared_ptr remote_socket_ptr = std::make_shared<tcp::socket>(io_context);
        asio::error_code connect_error;
        socks_spec::reply_code reply_code = socks_spec::reply_code::SUCCEEDED;
        try {
            co_await remote_socket_ptr->async_connect(endpoint,asio::use_awaitable);
        } catch (const asio::system_error& error) {
            switch (error.code().value()) {
                case asio::error::connection_refused: {         
                    reply_code = reply_code::CONNECTION_REFUSED;            
                    co_return;
                }
                case asio::error::network_unreachable: {
                    reply_code = reply_code::NETWORK_UNREACHABLE;            
                    co_return;
                }
                case asio::error::host_unreachable: {
                    reply_code = reply_code::HOST_UNREACHABLE;            
                    co_return;
                }
                case asio::error::timed_out: {
                    reply_code = reply_code::TTL_EXPIRED;            
                    co_return;
                }
                case asio::error::access_denied:        
                case asio::error::fault:{
                    reply_code = reply_code::RULESET_NOT_ALLOWED;            
                    co_return;
                }
                case asio::error::operation_not_supported:{
                    reply_code = reply_code::COMMAND_NOT_SUPPORTED;
                    co_return;
                }
                case asio::error::address_family_not_supported:{       
                    reply_code = reply_code::ADDR_TYPE_NOT_SUPPORTED;            
                    co_return;
                }
                default: {            
                    reply_code = reply_code::GENERAL_FAILURE;            
                    co_return;
                }
            }
        }
        tcp::endpoint connected_endpoint = remote_socket_ptr->remote_endpoint();
        asio::ip::address connected_address = connected_endpoint.address();
        socks_spec::address_type connected_address_type;
        if (connected_address.is_v4()) {
            connected_address_type = socks_spec::address_type::IPV4;
        } else if (connected_address.is_v6()) {
            connected_address_type = socks_spec::address_type::IPV6;
        } else {
            throw std::logic_error("connected address not specified");
        }
        socks::reply reply(
            socks_spec::version::VERSION5,
            reply_code,
            connected_address_type,
            connected_address.to_string(),
            connected_endpoint.port());
        co_await connection_ptr->write_bytes(asio::buffer(reply.buffer));
        
        const auto relay_client_to_remote = [connection_ptr,remote_socket_ptr] () -> asio::awaitable<void> {
            std::array<uint8_t, socks::RELAY_BUFFER_SIZE> relay_buffer;
            try {
                while (true) {
                    size_t written_size = co_await connection_ptr->socket().async_read_some(asio::buffer(relay_buffer),asio::use_awaitable);
                    co_await asio::async_write(*remote_socket_ptr,asio::buffer(relay_buffer,written_size),asio::use_awaitable);
                }
            } catch (std::system_error& e) {
                switch (e.code().value()) {
                    case asio::error::operation_aborted: {
                        logger.debug("Client->Remote relay cancelled normally");
                        break;
                    }
                    case asio::error::eof: {
                        logger.debug("Client disconnected");
                        break;
                    }
                    default: {
                        logger.debug("Client->Remote relay error: {}", e.what());
                        break;
                    }
                }

            } catch (...) {
                logger.error("unknown error");
                throw;
            }
        };
        const auto relay_remote_to_client = [connection_ptr,remote_socket_ptr] () -> asio::awaitable<void> {
            std::array<uint8_t, socks::RELAY_BUFFER_SIZE> relay_buffer;
            try {
                while (true) {
                    size_t written_size = co_await remote_socket_ptr->async_read_some(asio::buffer(relay_buffer),asio::use_awaitable);
                    co_await connection_ptr->write_bytes(asio::buffer(relay_buffer,written_size));
                }
            } catch (std::system_error& e) {
                switch (e.code().value()) {
                    case asio::error::operation_aborted: {
                        logger.debug("Remote->Client relay cancelled normally");
                        break;
                    }
                    case asio::error::eof: {
                        logger.debug("Remote disconnected");
                        break;
                    }
                    default: {
                        logger.warn("Remote->Client relay error: {}", e.what());
                        break;
                    }
                }
            } catch (...) {
                    logger.error("unknown error");
                    throw;
            }
        };
        asio::co_spawn(io_context,std::move(relay_client_to_remote),asio::detached);
        asio::co_spawn(io_context,std::move(relay_remote_to_client),asio::detached);
    }
