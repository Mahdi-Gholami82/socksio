#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <format>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace argp {
    class argument;
    inline size_t get_argument_hash(const argp::argument& arg);
}

namespace std {
    template<>
    struct hash<argp::argument> {
        size_t operator()(const argp::argument& arg) const {
            return get_argument_hash(arg);
        }
    };
}

namespace argp {
    class argument;


    namespace {
        template<typename Tp, typename Up>
        concept is_same_p = std::is_same_v<std::remove_cv_t<Tp>, Up>;

        template<typename T>
        concept is_argument = 
            is_same_p<T, int> ||
            is_same_p<T, long> ||
            is_same_p<T, long long> ||
            is_same_p<T, float> ||
            is_same_p<T, double> ||
            is_same_p<T, long double> ||
            is_same_p<T, bool>;

        inline bool is_ascii_alpha_(char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        } 

        constexpr std::string_view name_pattern = R"([a-zA-Z]+(?:[a-zA-Z]*-[a-zA-Z]+)*)";
        const std::regex full_name_re_pattern("^--{0,2}(" + std::string(name_pattern) + ")$");

        template<typename NumType>
        requires is_argument<NumType>
        NumType to_integral_value_(std::string& text, size_t* idx,int base) = delete;
        
        template<>
        int to_integral_value_<int>(std::string& text, size_t* idx,int base) {
            return std::stoi(text,idx,base);
        }        
        template<>
        long to_integral_value_<long>(std::string& text, size_t* idx,int base) {
            return std::stol(text,idx,base);
        }        
        template<>
        long long to_integral_value_<long long>(std::string& text, size_t* idx,int base) {
            return std::stoll(text,idx,base);
        }        
        template<>
        float to_integral_value_<float>(std::string& text, size_t* idx,int base) {
            return std::stof(text,idx);
        }        
        template<>
        double to_integral_value_<double>(std::string& text, size_t* idx,int base) {
            return std::stod(text,idx);
        }
        template<>
        long double to_integral_value_<long double>(std::string& text, size_t* idx,int base) {
            return std::stold(text,idx);
        }

        template<typename NumType>
        requires is_argument<NumType>
        NumType to_integral_(std::string text,int base = 10) {
            size_t index;
            NumType result = to_integral_value_<NumType>(text,&index,base);
            if (index != text.length()) {
                throw std::runtime_error("wrong argument");
            }
            return result;
        }
            
    }

    class argument_validation_error : std::runtime_error {
        public:
            argument_validation_error(
                const std::string &message,
                const std::vector<std::string> invalid_arguments)
                : std::runtime_error(message),
                    invalid_arguments(invalid_arguments) {};
            const std::vector<std::string> invalid_arguments;
    };

    class argument {
        public:
            argument(std::string name, std::string description = "",std::optional<char> name_char = std::nullopt)
                : name(std::move(name)), name_char(name_char.has_value() ? *name_char : this->name[0]),description(std::move(description)) {
                        assert(std::regex_match(
                        this->name, std::regex(std::string(name_pattern))) &&
                    "Only alphabetic characters and (-) allowed in argument name");
                    if (name_char.has_value()) {
                        assert(is_ascii_alpha_(this->name_char.value()) &&
                                "Only alphabetic characters and (-) allowed in argument name");
                    }
                    }
            const std::string name;
            const std::optional<char> name_char;
            const std::string description;

            bool operator==(const argument& other) const {
                return name == other.name;
            }

        private:

    };

    inline size_t get_argument_hash(const argp::argument& arg) {
        return std::hash<std::string>()(arg.name);
    }

    class argument_list {
        public:
            argument_list(int argc,char* argv[]) 
                : values(&argv[1], &argv[argc]) {}
            std::unordered_set<argument> arguments; 
            std::vector<std::string> values;


            inline std::optional<std::string> get(argument argument) {
                arguments.insert(argument);
                std::vector<std::string>::const_iterator argument_iter = find_argument_(argument);
                if (argument_iter != values.end()) {
                    auto next = values.erase(argument_iter);
                    if (next != values.end()) {
                        std::string value = *next;
                        values.erase(next);
                        return value;
                    }
                }
                return std::nullopt;
            }

            template<typename NumType = int>
            requires is_argument<NumType>
            inline std::optional<NumType> get_num(argument argument,int base = 10) {
                std::optional<std::string> argument_value = get(argument);
                if (argument_value.has_value()) {
                    return to_integral_<NumType>(argument_value.value());
                }
                return std::nullopt;
            }

            inline bool is_specified(argument argument) {
                std::vector<std::string>::const_iterator argument_iter = find_argument_(argument);
                if (argument_iter != values.end()) {
                    values.erase(argument_iter);
                    return true;
                }
                return false;
            }

            std::string generate_help() {
                std::string help_text = "";
                for (argument arg : arguments) {
                    help_text += std::format("\t-{}, --{}\n\t\t{}\n",arg.name_char.value(),arg.name,arg.description);
                }
                return help_text;
            }

            void validate() {
                if (!values.empty()) {
                    throw argument_validation_error("Invalid arguments",values);
                }
            }

        private:
            std::vector<std::string>::const_iterator find_argument_(argument argument) {
                return  std::find_if(values.begin(),values.end(),[&argument] (std::string value) {
                    bool has_match = std::regex_match(value,full_name_re_pattern);
                    if (has_match) {
                        std::sregex_iterator regex_iterator(value.begin(),value.end(),full_name_re_pattern);
                        std::smatch match = *regex_iterator;
                        std::string name_result = match[1];
                        return name_result == argument.name || argument.name_char.has_value() && (name_result.length() == 1 && name_result[0] == argument.name_char.value());
                    }
                    return false;
                });
            }
            
    };
}

