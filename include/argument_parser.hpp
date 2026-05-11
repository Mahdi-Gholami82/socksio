#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <format>
#include <functional>
#include <optional>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <vector>
#include <print>

namespace argp {
    class argument;
    inline size_t get_argument_hash(const argp::argument& arg);

    class argument_validation_error : public std::runtime_error {
    public:
      argument_validation_error(
          const std::string &message,
          const std::vector<std::string> invalid_arguments)
          : std::runtime_error(message),
            invalid_arguments(invalid_arguments) {};
      const std::vector<std::string> invalid_arguments;
    };

    class invalid_argument_value : public std::runtime_error {
    private:
        std::string msg_;
    
    public:
      invalid_argument_value(const std::string &message,
                             const std::string target_argument = "",
                            const std::string invalid_value = "")
          : std::runtime_error(message),
            invalid_value(std::move(invalid_value)), target_argument(std::move(target_argument)){
                msg_ = std::format("{}\n\nfor argument: {}\nvalue: {}\n ",message,target_argument,invalid_value);
            }
      const std::string invalid_value;
      const std::string target_argument;
      inline const char* what() const noexcept override {
            return msg_.c_str();
      }
    };
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

        template<typename T>
        concept is_num = 
            std::is_same_v<T, int> ||
            std::is_same_v<T, long> ||
            std::is_same_v<T, long long> ||
            std::is_same_v<T, float> ||
            std::is_same_v<T, double> ||
            std::is_same_v<T, long double>;

        template<typename T>
        concept is_argument = std::is_same_v<T, std::string> || is_num<T>;

        inline bool is_ascii_alpha_(char c) {
            return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        } 

        constexpr std::string_view name_pattern = R"([a-zA-Z]+(?:[a-zA-Z]*-[a-zA-Z]+)*)";
        const std::regex full_name_re_pattern("^--?(" + std::string(name_pattern) + ")$");

        /// used to convert string to number
        template<typename NumType>
        requires is_num<NumType>
        NumType to_numeric_value_(std::string& text, size_t* idx,int base) = delete;
        
        template<>
        int to_numeric_value_<int>(std::string& text, size_t* idx,int base) {
            return std::stoi(text,idx,base);
        }        
        template<>
        long to_numeric_value_<long>(std::string& text, size_t* idx,int base) {
            return std::stol(text,idx,base);
        }        
        template<>
        long long to_numeric_value_<long long>(std::string& text, size_t* idx,int base) {
            return std::stoll(text,idx,base);
        }        
        template<>
        float to_numeric_value_<float>(std::string& text, size_t* idx,int base) {
            return std::stof(text,idx);
        }        
        template<>
        double to_numeric_value_<double>(std::string& text, size_t* idx,int base) {
            return std::stod(text,idx);
        }
        template<>
        long double to_numeric_value_<long double>(std::string& text, size_t* idx,int base) {
            return std::stold(text,idx);
        }

        template<typename NumType>
        requires is_num<NumType>
        NumType to_integral_(std::string text,int base = 10) {
            size_t index;
            NumType result = to_numeric_value_<NumType>(text,&index,base);
            if (index != text.length()) {
                throw std::logic_error("argument is not pure integral value");
            }
            return result;
        }
            
    }

    /// Stores informations about an argument
    class argument {
        public:
            argument(std::string name, std::string description = "",std::optional<char> name_char = std::nullopt)
                : name(std::move(name)), name_char(name_char.has_value() ? *name_char : this->name[0]),description(std::move(description)) {
                        assert(std::regex_match(
                        this->name, std::regex(std::string(name_pattern))) &&
                    "Only alphabetic characters and (-) allowed in argument name");
                    if (name_char.has_value()) {
                        assert(is_ascii_alpha_(this->name_char.value()) &&
                                "Only alphabetic characters allowed in argument name_char");
                    }
                    }
            const std::string name;
            const std::optional<char> name_char;
            const std::string description;

            bool operator==(const argument& other) const {
                return name == other.name;
            }

    };

    inline size_t get_argument_hash(const argp::argument& arg) {
        return std::hash<std::string>()(arg.name);
    }

    /// used to manage and store arguments
    class manager {
        public:
            manager(int argc, char *argv[]) : values(&argv[1], &argv[argc]) {}
            std::unordered_set<argument> arguments;
            std::vector<std::string> values;
            std::vector<std::string>::const_iterator
            find_argument(argument argument) {
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

            /// Checks the existance argument and its value 
            void validate_valued_argument(std::vector<std::string>::const_iterator iterator) {
                auto next = iterator + 1;
                if (next == values.cend()) {
                    throw invalid_argument_value("No value provided",*iterator,"");
                }
                if (std::regex_match(*next,full_name_re_pattern)) {
                    throw invalid_argument_value("Invalid value / no value provided",*iterator,*next);
                }
            }
            
            /// Checks if argument is specified
            inline bool is_specified(const argument& argument) {
                std::vector<std::string>::const_iterator argument_iter = find_argument(argument);
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
            
            /// Checks if there are no arguments that havent been parsed
            void validate() {
                if (!values.empty()) {
                    throw argument_validation_error("Invalid arguments",values);
                }
            }
    };

    template<typename T>
    struct result {
        template<typename U = T>
        result(const U& value)
            : value(value) {}

        template<typename U = T>
        result(const U& value, std::string captured_from)
            : value(value),
            captured_from(std::move(captured_from)) {}
            
        T value;
        std::string captured_from;
        T operator*() { return value; }

    };

    /// Template used to create a subclass to specify program arguments 
    class args_template {
        public:
            args_template(int argc,char* argv[]) : manager_(argc, argv){}

            /// Process of extracting arguments
            /// Must be inside a try catch block to capture any exceptions
            inline void extract() {
                initiate(); 
                run_jobs();
                manager_.validate();
            }

        private:

            /// The actual process of parsing each argument is stored here as functions
            std::vector<std::function<void()>> jobs_ = {};

            /// Parses the arguments
            inline void run_jobs() {
                for (auto& job : jobs_) {
                    job();
                }
                jobs_.clear();
            }

        protected:
            manager manager_;

            /// Must be overridden, we define our arguments here
            virtual void initiate() = 0;

            /// define argument to be parsed as strings
            inline void define_arg(result<std::string>& arg_variable,const argument argument) {
                manager_.arguments.insert(argument);
                jobs_.push_back([&arg_variable,argument = std::move(argument),this] () {
                    std::vector<std::string>::const_iterator argument_iter = manager_.find_argument(argument);
                    if (argument_iter != manager_.values.end()) {
                        arg_variable.captured_from = *argument_iter;
                        manager_.validate_valued_argument(argument_iter);
                        auto next = argument_iter + 1;
                        std::string value = *next;
                        next = manager_.values.erase(argument_iter);
                        manager_.values.erase(next);
                        arg_variable.value = value;
                    }
                });
            }

            /// define argument to be parsed as numbers
            template<typename T>
            requires is_num<T>
            void define_arg(result<T>& arg_variable,const argument argument,int base = 10) {
                manager_.arguments.insert(argument);
                jobs_.push_back([&arg_variable,argument = std::move(argument),base,this] () {
                    std::vector<std::string>::const_iterator argument_iter = manager_.find_argument(argument);
                    if (argument_iter != manager_.values.end()) {
                        arg_variable.captured_from = *argument_iter;
                        manager_.validate_valued_argument(argument_iter);
                        auto next = argument_iter + 1;
                        T value;
                        try {
                            value = to_integral_<T>(*next);
                        } catch (const std::logic_error&) {
                            throw invalid_argument_value("Failed to parse argument value to int",*argument_iter,*next);
                        }
                        next = manager_.values.erase(argument_iter);
                        manager_.values.erase(next);
                        arg_variable.value = value;
                    }

                });
            }

            /// defines argument that doesnt get any value, and checks if specified
            inline void define_arg(result<bool>& arg_variable,const argument argument) {
                manager_.arguments.insert(argument);
                arg_variable = manager_.is_specified(argument);
            }

    };
}

