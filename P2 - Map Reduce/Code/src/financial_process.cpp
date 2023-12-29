#include "financial_process.hpp"

Financial::Financial(Logger* log, const std::string& name,
                     const std::vector<std::string>& buildingNames) {
    log_ = log;
    name_ = name;
    log_->info("Financial process started. Name: " + name_);
    data_ = new Bill(billCSVPath);
    connect(buildingNames);
}

void Financial::connect(const std::vector<std::string>& buildingNames) {
    for (const auto& name : buildingNames) buildings_.push_back(new Reduce(name, DEFAULT_ID));
}

void Financial::run() {
    while (true) {
        std::vector<std::string> args;
        std::string cmd = readfd(STDIN_FILENO, args);
        handleCMD(cmd, args);
    }
}

void Financial::handleCMD(const std::string& cmd, const std::vector<std::string>& args) {
    if (cmd == MSG_REPORT_BILL) {
        log_->info("Handling command: " + cmd);
        bill(args[0], args[1], args[2]);
        log_->info("Command handled: " + cmd);
    } else {
        log_->error("Unknown command");
    }
}

void Financial::bill(std::string buildingName, std::string resource, std::string month) {
    int buildingIndex = findBuilding(buildingName);
    std::cout << "fuck u\n";  // don't touch this or it will break :)
    std::cout << buildingName << ' ' << resource << ' ' << month << std::endl;
    std::vector<int> params = data_->calc(stoi(month));
    int finalParam = 0;

    std::map<std::string, int> resourceMap = {
        {WATER, params[0]}, {GAS, params[1]}, {ELECTRICITY, params[2]}};

    auto it = resourceMap.find(resource);
    if (it != resourceMap.end())
        finalParam = it->second;
    else
        throw std::runtime_error("Unknown resource: " + resource);

    buildings_[buildingIndex]->sendMessage(MSG_REPORT_DATA, {std::to_string(finalParam)});
}

int Financial::findBuilding(const std::string& name) {
    auto it = std::find_if(buildings_.begin(), buildings_.end(),
                           [&](auto& building) { return name == building->getName(); });

    if (it != buildings_.end()) {
        int index = std::distance(buildings_.begin(), it);
        return index;
    } else {
        log_->error("Building not found: " + name);
        return -1;
    }
}
