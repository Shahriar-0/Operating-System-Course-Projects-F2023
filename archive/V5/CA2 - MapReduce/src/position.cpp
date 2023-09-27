#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "consts.hpp"
#include "csv.hpp"
#include "logger.hpp"
#include "strutils.hpp"

using namespace std::string_literals;
namespace fs = std::filesystem;

Logger lg("reduce");

void saveData(std::string data, std::vector<int>& ages) {
    auto ages_str = strutils::split(data, ',');
    for (auto& age : ages_str) {
        ages.push_back(std::stoi(age));
    }
}

float avgAge(const std::vector<int>& ages) {
    int sum = 0;
    for (auto& age : ages) {
        sum += age;
    }
    return (float)sum / ages.size();
}

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        lg.error("Wrong number of arguments");
        return 1;
    }

    std::vector<int> ages;

    for (const auto& entry : fs::directory_iterator(consts::PIPES_PATH)) {
        std::string pipe_name = entry.path().filename();
        if (pipe_name.find(argv[1]) == 0) {
            int fd = open(entry.path().c_str(), O_RDONLY);
            if (fd == -1) {
                lg.error("Can't open pipe: "s + entry.path().c_str());
                return 1;
            }

            char buffer[consts::BUFF_SIZE];
            int bytes_read = read(fd, buffer, consts::BUFF_SIZE);
            close(fd);
            if (bytes_read == -1) {
                lg.error("Can't read from pipe");
                return 1;
            }
            std::string data(buffer, bytes_read);

            saveData(data, ages);
        }
    }

    std::cout << argv[1] << " min age: " << *std::min_element(ages.begin(), ages.end()) << std::endl;
    std::cout << argv[1] << " max age: " << *std::max_element(ages.begin(), ages.end()) << std::endl;
    std::cout << argv[1] << " avg age: " << std::setprecision(1) << std::fixed << avgAge(ages) << std::endl;
    std::cout << argv[1] << " count: " << ages.size() << std::endl;

    return EXIT_SUCCESS;
}