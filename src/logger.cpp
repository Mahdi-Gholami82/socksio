#include <sstream>
#include <chrono>
#include "logger.hpp"

const std::string get_log_name(log_level level) {
    switch (level) {
    case log_level::debug:
        return "debug";
    case log_level::info:
        return "info";
    case log_level::warn:
        return "warn";
    case log_level::error:
        return "error";
    case log_level::critical:
        return "critical";
    default:
        return "";
    }
}

constexpr std::string colors::from_log_level(log_level level) noexcept {
    switch (level) {
    case log_level::debug:
        return green;
    case log_level::info:
        return cyan;
    case log_level::warn:
        return yellow;
    case log_level::error:
        return red;
    case log_level::critical:
        return purple;
    default:
        return green;
    } 
}
void simple_logger::set_log_level(log_level level) noexcept {
    default_log_level = level;
}
std::string simple_logger::make_base_(log_level level) {
    std::stringstream date_time;
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    date_time << std::put_time(std::localtime(&now), "%F %T");
    return std::format("{}[{}]{} {}", colors::from_log_level(level),get_log_name(level), colors::reset, date_time.str());
}

simple_logger logger(log_level::debug);