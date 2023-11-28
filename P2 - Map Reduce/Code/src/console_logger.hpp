#ifndef CONSOLE_LOGGER_HPP_INCLUDE
#define CONSOLE_LOGGER_HPP_INCLUDE

#include "utils.hpp"
#include "consts.hpp"
#include "logger.hpp"

class ConsoleLogger : public Logger {
public:
    ConsoleLogger(const std::string& name) : Logger(name) {}
    virtual void info(const std::string& msg) override {
        std::cout << BLUE << name_ << ": " << msg << RESET << std::endl;
    }
    virtual void error(const std::string& msg) override {
        std::cerr << RED << name_ << ": " << msg << RESET << std::endl;
    }
};

#endif