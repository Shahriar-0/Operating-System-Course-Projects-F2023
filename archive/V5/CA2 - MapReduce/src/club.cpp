#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "consts.hpp"
#include "csv.hpp"
#include "logger.hpp"
#include "strutils.hpp"

using namespace std::string_literals;
namespace fs = std::filesystem;

Logger lg("club");

bool isInPosWanted(std::string pos, const std::vector<std::string>& pos_wanted) {
    for (auto& p : pos_wanted) {
        if (p == pos)
            return true;
    }
    return false;
}

std::string getFileName(std::string path) {
    size_t pos = path.find_last_of("/");
    std::string lastPart = path.substr(pos + 1);

    pos = lastPart.rfind(".");
    if (pos != std::string::npos)
        lastPart = lastPart.substr(0, pos);

    return lastPart;
}

int main(int argc, const char* argv[]) {
    if (argc != 4) {
        lg.error("Wrong number of arguments");
        return 1;
    }

    int fd0 = atoi(argv[2]);
    int fd1 = atoi(argv[3]);

    close(fd1);
    char buffer[consts::BUFF_SIZE];
    int bytes_read = read(fd0, buffer, consts::BUFF_SIZE);
    close(fd0);
    if (bytes_read == -1) {
        lg.error("Can't read from pipe");
        return 1;
    }

    std::vector<std::string> pos_wanted = strutils::split(buffer, ",");

    Csv csv(argv[1]);
    csv.readfile();
    auto tbl = csv.get();

    std::map<std::string, std::vector<std::string>> pos_to_ages;

    for (auto row : tbl) {
        if (isInPosWanted(row[1], pos_wanted)) {
            pos_to_ages[row[1]].push_back(row[2]);
        }
    }

    for (auto& [pos, ages] : pos_to_ages) {
        std::string pipe_name = std::string(consts::PIPES_PATH) + "/" + pos + "_" + getFileName(argv[1]);
        if (mkfifo(pipe_name.c_str(), 0666) == -1) {
            lg.error("Can't create pipe");
            return 1;
        }
    }

    sleep(1.5);

    for (auto& [pos, ages] : pos_to_ages) {
        std::string pipe_name = std::string(consts::PIPES_PATH) + "/" + pos + "_" + getFileName(argv[1]);
        int fd = open(pipe_name.c_str(), O_WRONLY);
        if (fd == -1) {
            lg.error("Can't open pipe: "s + pipe_name);
            return 1;
        }
        std::string data = strutils::join(ages, ",");
        write(fd, data.c_str(), data.size());
        close(fd);
    }


    return EXIT_SUCCESS;
}