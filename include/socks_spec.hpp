#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>

namespace socks_spec {
    template<typename E>concept is_scoped_enum_with_type_uint8_t = std::is_scoped_enum_v<E> && std::is_same_v<std::underlying_type_t<E>, uint8_t>;

    #define DEF_SOCKS_ENUM_OPERATORS(EnumType) \
        static_assert(is_scoped_enum_with_type_uint8_t<EnumType>, "EnumType must be an enum class"); \
        inline bool operator==(uint8_t val, EnumType e) { \
            return val == static_cast<uint8_t>(e); \
        } \
        inline bool operator==(EnumType e, uint8_t val) { \
            return val == e; \
        } \
        inline bool operator!=(uint8_t val, EnumType e) { \
            return !(val == e); \
        } \
        inline bool operator!=(EnumType e, uint8_t val) { \
            return !(val == e); \
        }
    
    template<typename EnumType> requires is_scoped_enum_with_type_uint8_t<EnumType>
    constexpr uint8_t to_underlying(EnumType e) noexcept {
        return static_cast<std::underlying_type_t<EnumType>>(e);
    }

    constexpr uint8_t ZERO = 0x00;

    enum class version : uint8_t {
        RESERVED = 0x00,
        VERSION4 = 0x04,
        VERSION5 = 0x05
    };
    const std::unordered_set<version> valid_versions = {version::VERSION4,version::VERSION5};
    DEF_SOCKS_ENUM_OPERATORS(version)

    inline version get_version(uint8_t value) {
        auto it = std::find_if(valid_versions.begin(),valid_versions.end(),[&value](version e){
            return value == e;
        });
        if (it != valid_versions.end()) {
            return *it;
        }
        throw std::runtime_error("Invalid version");
    }

    enum class method : uint8_t {
        NO_AUTHENTICATION_REQUIRED = 0x00,
        GSSAPI = 0x01,
        USERNAME_PASSWORD = 0x02,
        IANA_ASSIGNED_MIN = 0x03,
        IANA_ASSIGNED_MAX = 0x7F,
        RESERVED_PRIVATE_MIN = 0x80,
        RESERVED_PRIVATE_MAX = 0xFE,
        NO_ACCEPTABLE_METHODS = 0xFF
    };
    const std::unordered_set<method> valid_methods = {method::NO_AUTHENTICATION_REQUIRED};
    DEF_SOCKS_ENUM_OPERATORS(method)

    inline method get_method(uint8_t value) {
        auto it = std::find_if(valid_methods.begin(),valid_methods.end(),[&value](method e){
            return value == e;
        });
        if (it != valid_methods.end()) {
            return *it;
        }
        throw std::runtime_error("Invalid method");
    }

    enum class address_type : uint8_t {
        IPV4 = 0x01,
        IPV6 = 0x04,
        DOMAIN = 0x03,
        EMPTY = 0x00
    };
    const std::unordered_set<address_type> valid_address_types = {address_type::IPV4,address_type::IPV6,address_type::DOMAIN};
    DEF_SOCKS_ENUM_OPERATORS(address_type)

    inline address_type get_address_type(uint8_t value) {
        auto it = std::find_if(valid_address_types.begin(),valid_address_types.end(),[&value](address_type e){
            return value == e;
        });
        if (it != valid_address_types.end()) {
            return *it;
        }
        throw std::runtime_error("Invalid address type");
    }

    enum class command : uint8_t {
        CONNECT = 0x01,
        BIND = 0x02,
        UDP_ASSOCIATE = 0x03
    };
    const std::unordered_set<command> valid_commands = {command::CONNECT};
    DEF_SOCKS_ENUM_OPERATORS(command)

    inline command get_command(uint8_t value) {
        auto it = std::find_if(valid_commands.begin(),valid_commands.end(),[&value](command e){
            return value == e;
        });
        if (it != valid_commands.end()) {
            return *it;
        }
        throw std::runtime_error("Invalid command");
    }

    enum class reply_code : uint8_t {
        SUCCEEDED = 0x00,
        GENERAL_FAILURE = 0x01,
        RULESET_NOT_ALLOWED = 0x02,
        NETWORK_UNREACHABLE = 0x03,
        HOST_UNREACHABLE = 0x04,
        CONNECTION_REFUSED = 0x05,
        TTL_EXPIRED = 0x06,
        COMMAND_NOT_SUPPORTED = 0x07,
        ADDR_TYPE_NOT_SUPPORTED = 0x08
    };
    DEF_SOCKS_ENUM_OPERATORS(reply_code)

    #undef SOCKS_ENUM_OPERATORS
}