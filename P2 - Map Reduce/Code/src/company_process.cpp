#include "company_process.hpp"

Company::Company(const char* buildingsPath, Logger* log) {
    log_ = log;
    log_->info("Company initialization");

    try {
        std::vector<std::string> buildingNames = getBuildings(buildingsPath);
        createBuildings(buildingNames, buildingsPath);
        createFinancialUnit(buildingNames);
        emptyLog();

        log_->info("Company initialization completed successfully");
    } catch (const std::exception& e) {
        log_->error("Company initialization failed: " + std::string(e.what()));
        throw;
    }
}

std::vector<std::string> Company::getBuildings(const char* path) {
    std::vector<std::string> names = GetFilesOfDirectory(path);
    if (names.empty()) throw std::runtime_error("No buildings found in the directory");
    return names;
}

void Company::createBuildings(const std::vector<std::string>& names, const char* buildingsPath) {
    for (const auto& name : names) {
        int pipeFds1[2], pipeFds2[2], pid;
        if (pipe(pipeFds1) == -1 || pipe(pipeFds2) == -1) {
            log_->error("Problem in creating pipe");
            throw std::runtime_error("Problem in creating pipe");
        }

        pid = fork();
        if (pid == 0) {
            setupUnnamedPipes(pipeFds1, pipeFds2);
            execl(EXE_BUILDING, EXE_BUILDING, buildingsPath, name.c_str(), nullptr);
            log_->error("Problem in executing building: " + name);
            throw std::runtime_error("Problem in executing building: " + name);
        } else if (pid > 0) {
            close(pipeFds1[0]);
            close(pipeFds2[1]);
            buildings_.push_back(new Map(name, pipeFds1[1], pipeFds2[0], pid));
        } else {
            log_->error("Problem in forking buildings");
            throw std::runtime_error("Problem in forking buildings");
        }
    }
}

void Company::createFinancialUnit(const std::vector<std::string>& buildingNames) {
    log_->info("Creating financial unit: " + std::string(FINANCIAL_NAME));

    int pipeFds1[2], pipeFds2[2], pid;
    if (pipe(pipeFds1) == -1 || pipe(pipeFds2) == -1) {
        log_->error("Problem in creating pipe");
        throw std::runtime_error("Problem in creating pipe");
    }

    pid = fork();
    if (pid == 0) {
        std::vector<std::string> tempArgs = {std::string(EXE_FINANCIAL),
                                             std::string(FINANCIAL_NAME)};
        tempArgs.insert(tempArgs.end(), buildingNames.begin(), buildingNames.end());
        char** args = vectorToCharArray(tempArgs);
        setupUnnamedPipes(pipeFds1, pipeFds2);
        execv(EXE_FINANCIAL, args);
        log_->error("Problem in executing financial unit: " + std::string(FINANCIAL_NAME));
        throw std::runtime_error("Problem in executing financial unit: " +
                                 std::string(FINANCIAL_NAME));
    } else if (pid > 0) {
        close(pipeFds1[0]);
        close(pipeFds2[1]);
        finance_ = new Map(FINANCIAL_NAME, pipeFds1[1], pipeFds2[0], pid);
    } else {
        log_->error("Problem in forking financial unit");
        throw std::runtime_error("Problem in forking financial unit");
    }
}

void Company::run() {
    while (true) {
        printPrompt();
        std::string input;
        std::getline(std::cin, input);
        std::vector<std::string> args;
        std::string cmd = decode(input, args);
        try {
            handleCMD(cmd, args);
        } catch (const std::exception& e) {
            log_->error("Error occurred while executing command: " + cmd + " with args " +
                        vectorToString(args) + " - " + std::string(e.what()));
        }
    }
}

void Company::printPrompt() {
    std::cout << YELLOW << "-----------------------------------------------------\n" << RESET
              << GREEN  << ">> " << RESET;
}

void Company::handleCMD(std::string& cmd, const std::vector<std::string>& args) {
    if (cmd == MSG_HELP || cmd[0] == MSG_HELP[0])
        showHelp();
    else if (cmd == MSG_REPORT || cmd[0] == MSG_REPORT[0])
        reportHandler();
    else if (cmd == MSG_BUILDINGS || cmd[0] == MSG_BUILDINGS[0])
        printBuildings();
    else if (cmd == MSG_EXIT || cmd[0] == MSG_EXIT[0])
        exit(0);
    else
        log_->error("unknown command " + cmd + " with args " + vectorToString(args));
}

void Company::printBuildings() {
    std::cout << "***************************************\n";
    std::cout << "Buildings:\n";
    for (long unsigned int i = 0; i < buildings_.size(); i++)
        std::cout << "\t" << BLUE << i << ". " << RESET << GREEN << buildings_[i]->getName()
                  << RESET << "\n";
    std::cout << "***************************************\n";
}

void Company::showHelp() {
    std::cout << GREEN << "\nYou can use the following commands:\n" << RESET;
    std::cout << RED << "1. report:\n" << RESET;
    std::cout << "\t- Usage: " << YELLOW << "report\n";
    std::cout << "\t- Description: " << RESET
              << "Generates a report based on the specified type for a specific building.\n";
    std::cout << "\t- Available report types:\n";
    std::cout << "\t\t- mean: " << CYAN
              << "Calculates the mean value of the specified resource for the given month.\n"
              << RESET;
    std::cout << "\t\t- total: " << CYAN
              << "Calculates the total value of the specified resource for the given month.\n"
              << RESET;
    std::cout << "\t\t- peak: " << CYAN
              << "Finds the peak value of the specified resource for the given month.\n"
              << RESET;
    std::cout << "\t\t- difference: " << CYAN
              << "Calculates the difference between the peak and the mean value of the specified "
                 "resource for the given month.\n"
              << RESET;
    std::cout << "\t\t- bill: " << CYAN
              << "Calculates the bill for the specified resource for the given month.\n"
              << RESET;
    std::cout << "\n";
    std::cout << RED << "2. help:\n" << RESET;
    std::cout << "\t- Usage: " << YELLOW << "help\n";
    std::cout << "\t- Description: " << RESET << "Displays this help file.\n";
    std::cout << RED << "3. building:\n" << RESET;
    std::cout << "\t- Usage: " << YELLOW << "building\n";
    std::cout << "\t- Description: " << RESET << "Displays the list of buildings.\n";
    std::cout << RED << "4. exit:\n" << RESET;
    std::cout << "\t- Usage: " << YELLOW << "exit\n";
    std::cout << "\t- Description: " << RESET << "Exits the program.\n";
}

void Company::reportHandler() {
    std::string type = prompt("Enter report type(mean, total, max, difference, bill): ");
    std::string name = prompt("Enter building name: ");
    int index = findBuilding(name);
    if (index == -1) {
        log_->error("Building " + std::to_string(index) + "' not found");
        return;
    }
    std::string resource = prompt("Enter resource name(Water, Gas, Electricity): ");
    std::string month = prompt("Enter month number: ");
    if (type == MSG_REPORT_FOR_MEAN || type[0] == MSG_REPORT_FOR_MEAN[0])
        mean(resource, month, index, name);
    else if (type == MSG_REPORT_FOR_TOTAL || type[0] == MSG_REPORT_FOR_TOTAL[0])
        total(resource, month, index, name);
    else if (type == MSG_REPORT_FOR_MAX || type[0] == MSG_REPORT_FOR_MAX[0])
        getMax(resource, month, index, name);
    else if (type == MSG_REPORT_FOR_DIFFERENCE || type[0] == MSG_REPORT_FOR_DIFFERENCE[0])
        diff(resource, month, index, name);
    else if (type == MSG_REPORT_BILL || type[0] == MSG_REPORT_BILL[0])
        bill(resource, month, index, name);
    else
        log_->error("Invalid report type: " + type);
}

void Company::mean(const std::string& resource, const std::string& month, int index, const std::string& name) {
    log_->info("Sending mean report request to building " + name);

    std::vector<std::string> resArgs = message(MSG_REPORT_FOR_MEAN, resource, month, index, name);

    log_->info("Received mean report response from building " + name);
    printRes("Mean is: ", resArgs[0]);
}

void Company::total(const std::string& resource, const std::string& month, int index, const std::string& name) {
    log_->info("Sending total report request to building " + name);

    std::vector<std::string> resArgs = message(MSG_REPORT_FOR_TOTAL, resource, month, index, name);

    log_->info("Received total report response from building " + name);
    printRes("Total cost is: ", resArgs[0]);
}

void Company::getMax(const std::string& resource, const std::string& month, int index, const std::string& name) {
    log_->info("Sending peak report request to building " + name);

    std::vector<std::string> resArgs = message(MSG_REPORT_FOR_MAX, resource, month, index, name);
    std::string maxHours = "";
    for (const auto& hour : resArgs) maxHours += (hour + " ");

    log_->info("Received peak report response from building " + name);
    printRes("Max hour is: ", maxHours);
}

void Company::diff(const std::string& resource, const std::string& month, int index, const std::string& name) {
    log_->info("Sending difference report request to building " + name);

    std::vector<std::string> resArgs = message(MSG_REPORT_FOR_DIFFERENCE, resource, month, index, name);

    log_->info("Received difference report response from building " + name);
    printRes("Difference is: ", resArgs[0]);
}

std::vector<std::string> Company::message(const std::string& msg, const std::string& resource,
                                          const std::string& month, int index, const std::string& name) {
    std::string buildingName = buildings_[index]->getName();
    std::vector<std::string> resArgs;
    buildings_[index]->sendMessage(msg, {resource, month});
    buildings_[index]->receiveMessage(resArgs);
    return resArgs;
}

void Company::bill(const std::string& resource, const std::string& month, int index, const std::string& name) {
    log_->info("Sending bill report request to building " + name);
    
    std::string buildingName = buildings_[index]->getName();
    std::string financialName = finance_->getName();

    buildings_[index]->sendMessage(MSG_REPORT_BILL, {resource, month});

    std::vector<std::string> financialArgs = {buildingName, resource, month};
    finance_->sendMessage(MSG_REPORT_BILL, financialArgs);
    
    std::vector<std::string> resArgs;
    buildings_[index]->receiveMessage(resArgs);

    log_->info("Received bill report response from building " + name);
    printRes("Bill cost is: ", resArgs[0]);
}

int Company::findBuilding(const std::string& name) {
    auto it = std::find_if(buildings_.begin(), buildings_.end(),
                           [&](const auto& building) { return building->getName() == name; });

    if (it != buildings_.end()) return std::distance(buildings_.begin(), it);
    return -1;
}
