#ifndef FINANCIAL_PROCESS_HPP_INCLUDE
#define FINANCIAL_PROCESS_HPP_INCLUDE

#include "console_logger.hpp"
#include "consts.hpp"
#include "file_console_logger.hpp"
#include "file_logger.hpp"
#include "logger.hpp"
#include "map.hpp"
#include "node.hpp"
#include "reduce.hpp"
#include "utils.hpp"

class Financial {
public:
    Financial(Logger* log, const std::string& name, const std::vector<std::string>& buildingNames);
    void run();

private:
    void connect(const std::vector<std::string>& buildingNames);
    void handleCMD(const std::string& cmd, const std::vector<std::string>& args);
    int findBuilding(const std::string& name);
    void bill(std::string buildingName, std::string resource, std::string m);

    Logger* log_;
    std::string name_;
    std::vector<Node*> buildings_;
    Bill* data_;
};

#endif
