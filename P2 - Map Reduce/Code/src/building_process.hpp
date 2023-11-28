#ifndef BUILDING_PROCESS_HPP_INCLUDE
#define BUILDING_PROCESS_HPP_INCLUDE

#include <string>
#include <vector>

#include "console_logger.hpp"
#include "consts.hpp"
#include "file_console_logger.hpp"
#include "file_logger.hpp"
#include "logger.hpp"
#include "map.hpp"
#include "node.hpp"
#include "reduce.hpp"
#include "utils.hpp"

class Building {
public:
    Building(Logger* log, std::string name, std::string path);
    void run();

private:
    void createResources();
    void connect();
    void handleCMD(const std::string& cmd, const std::vector<std::string>& args);
    int findResource(const std::string& name);

    void mean(std::string rt, std::string mNumber);
    void total(std::string rt, std::string mNumber);
    void diff(std::string rt, std::string mNumber);
    void bill(std::string rt, std::string mNumber);
    void getMax(std::string rt, std::string mNumber);
    void createResource(const std::string resource);
    std::vector<std::string> message(std::string msg, std::string rt, std::string mNumber);

    Logger* log_;
    std::string name_;
    std::string path_;
    std::vector<Node*> resources_;
    Node* finance_;
};

#endif
