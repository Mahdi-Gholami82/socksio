#pragma once

#include <format>
#include <iostream>
#include <map>
#include <print>
#include <ostream>
#include <string>
#include <utility>

using std::println;


enum class log_level {
    debug,
    info,
    warn,
    error,
    critical
};

extern const std::map<log_level, std::string> log_names;

const std::string get_log_name(log_level level);

struct colors{
    static constexpr std::string cyan = "\033[36m";
    static constexpr std::string green = "\033[32m";
    static constexpr std::string yellow = "\033[33m";
    static constexpr std::string red = "\033[1;31m";
    static constexpr std::string purple = "\033[35m";
    static constexpr std::string reset  = "\033[0m";

    static constexpr std::string from_log_level(log_level level) noexcept;
};

class simple_logger {
    public:
        simple_logger(log_level level) : default_log_level(level) {}

        void set_log_level(log_level level) noexcept;

        template <typename... Args>
        inline void log(log_level level,std::format_string<Args...> message, Args &&...args) {
            if (level < default_log_level) {
                return;
            }
            std::ostream& out = level > log_level::warn ? std::cerr : std::cout;
            std::println(out,"\r{} {}", make_base_(level),std::format(std::move(message),std::forward<Args>(args)...));
        }

        template <typename... Args>
        inline void debug(std::format_string<Args...> message, Args &&...args) {
            log(log_level::debug,std::move(message),std::forward<Args>(args)...);
        }
        template<typename ...Args>
        inline void info(std::format_string<Args...> message, Args&&... args) {
            log(log_level::info,std::move(message),std::forward<Args>(args)...);
        }
        template<typename ...Args>
        inline void warn(std::format_string<Args...> message, Args&&... args) {
            log(log_level::warn,std::move(message),std::forward<Args>(args)...);
        }
        template<typename ...Args>
        inline void error(std::format_string<Args...> message, Args&&... args) {
            log(log_level::error,std::move(message),std::forward<Args>(args)...);
        }
        template<typename ...Args>
        inline void critical(std::format_string<Args...> message, Args&&... args) {
            log(log_level::critical,std::move(message),std::forward<Args>(args)...);
        }
        template<typename ...Args>
        inline void raw(std::format_string<Args...> message, Args&&... args) {
            println(std::cout,std::move(message),std::forward<Args>(args)...);
        }
        template<typename ...Args>
        inline void raw_err(std::format_string<Args...> message, Args&&... args) {
            println(std::cerr,std::move(message),std::forward<Args>(args)...);
        }

    private:
        log_level default_log_level;

        std::string make_base_(log_level level);
};


extern simple_logger logger;