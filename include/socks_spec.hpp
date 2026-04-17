#pragma once

#include <array>
#include <cstdint>

namespace socks_spec{
    namespace version {
        constexpr uint8_t RESERVED = 0x00;
        constexpr uint8_t VERSION4 = 0x04;
        constexpr uint8_t VERSION5 = 0x05;
    };

    namespace method {
        constexpr uint8_t NO_AUTHENTICATION_REQUIRED = 0x00;
        constexpr uint8_t GSSAPI = 0x01;
        constexpr uint8_t USERNAME_PASSWORD = 0x02;
        constexpr uint8_t IANA_ASSIGNED_MIN = 0x03;
        constexpr uint8_t IANA_ASSIGNED_MAX = 0x7F;
        constexpr uint8_t RESERVED_PRIVATE_MIN = 0x80;
        constexpr uint8_t RESERVED_PRIVATE_MAX = 0xFE;
        constexpr uint8_t NO_ACCEPTABLE_METHODS = 0xFF;
        constexpr std::array<uint8_t,8> methods_list = {NO_AUTHENTICATION_REQUIRED,GSSAPI,USERNAME_PASSWORD,IANA_ASSIGNED_MIN,IANA_ASSIGNED_MAX,RESERVED_PRIVATE_MIN,RESERVED_PRIVATE_MAX,NO_ACCEPTABLE_METHODS};
    }

}

