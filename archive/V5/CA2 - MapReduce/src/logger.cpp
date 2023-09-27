#include "logger.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>

#include "colorprint.hpp"

Logger::Logger(std::string program) : program_(std::move(program)) {}

void Logger::error(const std::string& msg) {
    std::cerr << Color::RED << "[ERR:" << program_ << "] "
              << Color::RST << msg << '\n';
}

void Logger::warning(const std::string& msg) {
    std::cout << Color::YEL << "[WRN:" << program_ << "] "
              << Color::RST << msg << '\n';
}

void Logger::info(const std::string& msg) {
    std::cout << Color::BLU << "[INF:" << program_ << "] "
              << Color::RST << msg << '\n';
}

void Logger::perrno() {
    error(strerror(errno));
}
