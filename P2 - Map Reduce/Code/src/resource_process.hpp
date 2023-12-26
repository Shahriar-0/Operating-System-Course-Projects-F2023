#ifndef RESOURCE_PROCESS_HPP_INCLUDE
#define RESOURCE_PROCESS_HPP_INCLUDE

#include "console_logger.hpp"
#include "consts.hpp"
#include "file_console_logger.hpp"
#include "file_logger.hpp"
#include "logger.hpp"
#include "map.hpp"
#include "node.hpp"
#include "reduce.hpp"
#include "utils.hpp"

class Resource {
public:
    Resource(Logger* log, std::string name, std::string path);
    void run();

private:
    void handleCMD(const std::string& cmd, const std::vector<std::string>& args);
    void mean(int month);
    void total(int month);
    void getMax(int month);
    void diff(int month);
    void bill(int month, std::string rt, int p);

    Logger* log_;
    std::string name_;
    std::string CSVPath_;
    ResourceUsage* data_;
};

#endif