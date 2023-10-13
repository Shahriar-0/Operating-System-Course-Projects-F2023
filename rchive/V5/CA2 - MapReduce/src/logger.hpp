#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

class Logger {
public:
    Logger(std::string program);

    void error(const std::string& msg);
    void warning(const std::string& msg);
    void info(const std::string& msg);
    void perrno();

private:
    std::string program_;
};

#endif
