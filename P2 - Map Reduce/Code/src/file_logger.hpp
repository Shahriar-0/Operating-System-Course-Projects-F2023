#ifndef FILE_LOGGER_HPP_INCLUDE
#define FILE_LOGGER_HPP_INCLUDE

#include "consts.hpp"
#include "utils.hpp"
#include "logger.hpp"

class FileLogger : public Logger {
public:
    FileLogger(const std::string& name)
        : Logger(name), logFile_(LOG_FILE, std::ios::out | std::ios::app) {}

    ~FileLogger() { logFile_.close(); }

    virtual void info(const std::string& msg) override {
        logFile_ << INFO_LOG << " " << name_ << "': " << msg << std::endl;
    }

    virtual void error(const std::string& msg) override {
        logFile_ << ERROR_LOG << " " << name_ << "': " << msg << std::endl;
    }

private:
    std::fstream logFile_;
};

#endif