#ifndef LOGGER_HPP_INCLUDE
#define LOGGER_HPP_INCLUDE

#include "consts.hpp"

constexpr char ERROR_LOG[] = "ERROR";
constexpr char INFO_LOG[] = "INFO";

class Logger {
public:
    Logger(const std::string& name) : name_(name) {};
    virtual ~Logger() = default;
    virtual void info(const std::string& msg) = 0;
    virtual void error(const std::string& msg) = 0;

protected:
    std::string name_;
};

#endif
