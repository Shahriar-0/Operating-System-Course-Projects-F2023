#include "building_process.hpp"

Building::Building(Logger* log, std::string name, std::string path) {
    log_ = log;
    name_ = name;
    path_ = path;
    log_->info("Start working");
    createResources();
    connect();
}

void Building::createResource(const std::string resource) {
    int pipeFds1[2], pipeFds2[2], pid;
    if (pipe(pipeFds1) == -1 || pipe(pipeFds2) == -1) {
        throw new std::runtime_error("Failed to create pipe for resource: " + resource);
    }

    pid = fork();
    if (pid == 0) {
        setupUnnamedPipes(pipeFds1, pipeFds2);
        execl(EXE_RESOURCE, EXE_RESOURCE, std::string(path_ + "/" + name_).c_str(), name_.c_str(),
              resource.c_str(), nullptr);
        throw new std::runtime_error("Failed to execute resource: " + resource);
    } else if (pid > 0) {
        close(pipeFds1[0]);
        close(pipeFds2[1]);
        resources_.push_back(new Map(resource, pipeFds1[1], pipeFds2[0], pid));
        log_->info("Resource " + resource + " created successfully");
    } else {
        throw new std::runtime_error("Failed to fork resources for resource: " + resource);
    }
}

void Building::createResources() {
    createResource(WATER);
    createResource(GAS);
    createResource(ELECTRICITY);
}

void Building::connect() { finance_ = new Reduce(name_, DEFAULT_ID); }

void Building::run() {
    while (true) {
        std::vector<std::string> args;
        std::string cmd = readfd(STDIN_FILENO, args);
        handleCMD(cmd, args);
    }
}

void Building::handleCMD(const std::string& cmd, const std::vector<std::string>& args) {
    if (cmd == MSG_REPORT_FOR_MEAN) {
        log_->info("Calling mean function");
        mean(args[0], args[1]);
        log_->info("Mean function call completed");
    } else if (cmd == MSG_REPORT_FOR_TOTAL) {
        log_->info("Calling total function");
        total(args[0], args[1]);
        log_->info("Total function call completed");
    } else if (cmd == MSG_REPORT_FOR_PEAK) {
        log_->info("Calling getMax function");
        getMax(args[0], args[1]);
        log_->info("getMax function call completed");
    } else if (cmd == MSG_REPORT_FOR_DIFFERENCE) {
        log_->info("Calling diff function");
        diff(args[0], args[1]);
        log_->info("diff function call completed");
    } else if (cmd == MSG_REPORT_BILL) {
        log_->info("Calling bill function");
        bill(args[0], args[1]);
        log_->info("bill function call completed");
    } else {
        log_->error("unknown command");
    }
}

void Building::mean(std::string rt, std::string mNumber) {
    std::vector<std::string> resArgs = message(MSG_REPORT_FOR_MEAN, rt, mNumber);
    writefd(STDOUT_FILENO, MSG_RESPONSE, resArgs);
}

void Building::total(std::string rt, std::string mNumber) {
    std::vector<std::string> resArgs = message(MSG_REPORT_FOR_TOTAL, rt, mNumber);
    writefd(STDOUT_FILENO, MSG_RESPONSE, resArgs);
}

void Building::getMax(std::string rt, std::string mNumber) {
    std::vector<std::string> resArgs = message(MSG_REPORT_FOR_PEAK, rt, mNumber);
    writefd(STDOUT_FILENO, MSG_RESPONSE, resArgs);
}

void Building::diff(std::string rt, std::string mNumber) {
    std::vector<std::string> resArgs = message(MSG_REPORT_FOR_DIFFERENCE, rt, mNumber);
    writefd(STDOUT_FILENO, MSG_RESPONSE, resArgs);
}

void Building::bill(std::string rt, std::string mNumber) {
    std::vector<std::string> p;
    finance_->receiveMessage(p);
    int index = findResource(rt);
    resources_[index]->sendMessage(MSG_REPORT_BILL, {mNumber, rt, p[0]});
    std::vector<std::string> resArgs;
    resources_[index]->receiveMessage(resArgs);
    writefd(STDOUT_FILENO, MSG_RESPONSE, resArgs);
}

std::vector<std::string> Building::message(std::string msg, std::string rt,
                                           std::string mNumber) {
    int index = findResource(rt);
    resources_[index]->sendMessage(msg, {mNumber});
    std::vector<std::string> resArgs;
    resources_[index]->receiveMessage(resArgs);
    return resArgs;
}

int Building::findResource(const std::string& name) {
    auto it = std::find_if(resources_.begin(), resources_.end(),
                           [&](auto& resource) { return resource->getName() == name; });

    if (it != resources_.end()) return std::distance(resources_.begin(), it);
    return -1;
}
