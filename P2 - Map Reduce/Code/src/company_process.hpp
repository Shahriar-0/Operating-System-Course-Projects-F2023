#ifndef COMPANY_PROCESS_HPP_INCLUDE
#define COMPANY_PROCESS_HPP_INCLUDE

#include "console_logger.hpp"
#include "consts.hpp"
#include "file_console_logger.hpp"
#include "file_logger.hpp"
#include "logger.hpp"
#include "map.hpp"
#include "node.hpp"
#include "reduce.hpp"
#include "utils.hpp"

class Company {
public:
    Company(const char* buildingsPath, Logger* log);
    void run();

private:
    std::vector<std::string> getBuildings(const char* path);
    void printPrompt();
    void createBuildings(const std::vector<std::string>& names, const char* buildingsPath);
    void createFinancialUnit(const std::vector<std::string>& buildingNames);
    void handleCMD(std::string& cmd, const std::vector<std::string>& args);
    void showHelp();
    void reportHandler();
    int  findBuilding(const std::string& name);
    
    void getMax(const std::string& resource, const std::string& month, int index, const std::string& name);
    void mean(const std::string& resource, const std::string& month, int index, const std::string& name);
    void total(const std::string& resource, const std::string& month, int index, const std::string& name);
    void diff(const std::string& resource, const std::string& month, int index, const std::string& name);
    void bill(const std::string& resource, const std::string& month, int index, const std::string& name);
    void printBuildings();
    std::vector<std::string> message(const std::string& msg, const std::string& resource,
                                     const std::string& month, int index, const std::string& name);

    Logger* log_;
    std::vector<Node*> buildings_;
    Node* finance_;
};

#endif
