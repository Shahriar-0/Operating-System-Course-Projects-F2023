#ifndef FILE_CONSOLE_LOGGER_HPP_INCLUDE
#define FILE_CONSOLE_LOGGER_HPP_INCLUDE

#include "console_logger.hpp"
#include "consts.hpp"
#include "file_logger.hpp"

class FileConsoleLogger : public Logger {
public:
    FileConsoleLogger(const std::string& name)
        : Logger(name) {
        consolLogger_ = new ConsoleLogger(name);
        fileLogger_ = new FileLogger(name);
    }
    virtual void info(const std::string& msg) override {
        consolLogger_->info(msg);
        fileLogger_->info(msg);
    }
    virtual void error(const std::string& msg) override {
        consolLogger_->error(msg);
        fileLogger_->error(msg);
    }

private:
    ConsoleLogger* consolLogger_;
    FileLogger* fileLogger_;
};

#endif