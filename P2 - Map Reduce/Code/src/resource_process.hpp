#ifndef RESOURCE_PROCESS_HPP_INCLUDE
#define RESOURCE_PROCESS_HPP_INCLUDE

#include "node.hpp"
#include "reduce.hpp"
#include "map.hpp"
#include "console_logger.hpp"
#include "consts.hpp"
#include "file_console_logger.hpp"
#include "file_logger.hpp"
#include "logger.hpp"
#include "utils.hpp"

class Resource {
public:
    Resource(Logger* log, std::string name, std::string path);
    void run();

private:
    void handleCMD(const std::string& cmd, const std::vector<std::string>& args);
    void mean(int m);
    void total(int m);
    void getMax(int m);
    void diff(int m);
    void bill(int m, std::string resourceType, int p);

    Logger* log_;
    std::string name_;
    std::string CSVPath_;
    ResourceUsage* data_;
};

#endif