#include "resource_process.hpp"

Resource::Resource(Logger* log, std::string name, std::string path) {
    log_ = log;
    name_ = name;
    CSVPath_ = path + "/" + name + ".csv";
    log_->info("Start working");
    data_ = new ResourceUsage(CSVPath_);
}

void Resource::run() {
    while (true) {
        std::vector<std::string> args;
        std::string cmd = readfd(STDIN_FILENO, args);
        handleCMD(cmd, args);
    }
}

void Resource::handleCMD(const std::string& cmd, const std::vector<std::string>& args) {
    log_->info("Handling command: {}" + cmd);
    std::cout << "fuck u\n"; // don't touch this or it will break :)
    if (cmd == MSG_REPORT_FOR_MEAN) {
        log_->info("Calling mean function");
        mean(stoi(args[0]));
        log_->info("Mean function called");
    }
    else if (cmd == MSG_REPORT_FOR_TOTAL) {
        log_->info("Calling total function");
        total(stoi(args[0]));
        log_->info("Total function called");
    }
    else if (cmd == MSG_REPORT_FOR_MAX) {
        log_->info("Calling getMax function");
        getMax(stoi(args[0]));
        log_->info("getMax function called");
    }
    else if (cmd == MSG_REPORT_FOR_DIFFERENCE) {
        log_->info("Calling diff function");
        diff(stoi(args[0]));
        log_->info("diff function called");
    }
    else if (cmd == MSG_REPORT_BILL) {
        log_->info("Calling bill function");
        srand(time(NULL));
        int p = rand() % 12 + 1;
        bill(stoi(args[0]), args[1], p);
        log_->info("bill function called");
    }
    else 
        log_->error("Unknown command");
}

void Resource::mean(int month) {
    int re = data_->mean(month);
    writefd(STDOUT_FILENO, MSG_RESPONSE, {std::to_string(re)});
}

void Resource::total(int month) {
    int re = data_->mean(month) * 30;
    writefd(STDOUT_FILENO, MSG_RESPONSE, {std::to_string(re)});
}

void Resource::getMax(int month) {
    std::vector<int> tempRes = data_->maxUsage(month);
    std::vector<std::string> re;
    for (long unsigned int i = 0; i < tempRes.size(); i++) re.push_back(std::to_string(tempRes[i]));
    writefd(STDOUT_FILENO, MSG_RESPONSE, re);
}

void Resource::diff(int month) {
    int re = data_->diff(month);
    writefd(STDOUT_FILENO, MSG_RESPONSE, {std::to_string(re)});
}

void Resource::bill(int month, std::string rt, int p) {
    int re;

    if (rt == WATER)
        re = data_->water(month, p);
    else if (rt == GAS)
        re = data_->gas(month, p);
    else if (rt == ELECTRICITY)
        re = data_->electricity(month, p);
    else 
        log_->error("Unknown resource type");
    
    writefd(STDOUT_FILENO, MSG_RESPONSE, {std::to_string(re)});
}
