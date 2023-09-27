#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "consts.hpp"
#include "logger.hpp"
#include "strutils.hpp"

using namespace std::string_literals;
namespace fs = std::filesystem;

Logger lg("country");

int get_directory_files(std::string path, std::vector<fs::path>& files) {
    if (fs::exists(path) && fs::is_directory(path)) {
        fs::path directory_path = path;
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path());
            }
        }
    }
    else {
        lg.error("Could not open directory: "s + path);
        return 1;
    }
    return 0;
}

int main(int argc, const char* argv[]) {
    if (argc != 4) {
        lg.error("Wrong number of arguments");
        return 1;
    }

    std::vector<fs::path> clubs;

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

    if (get_directory_files(argv[1], clubs))
        return EXIT_FAILURE;

    // create unnamed pipes for clubs
    int club_pipes[clubs.size()][2];
    for (int i = 0; i < clubs.size(); i++) {
        if (pipe(club_pipes[i]) == -1) {
            lg.error("Could not create club pipe");
            return EXIT_FAILURE;
        }
    }

    // create process for each club
    for (int i = 0; i < clubs.size(); i++) {
        int pid = fork();

        if (pid < 0) {
            lg.error("Could not create child process for position: "s + clubs[i].c_str());
        }
        else if (pid == 0) { // Child process
            char argv[3][256];
            sprintf(argv[0], "%s", clubs[i].c_str());
            sprintf(argv[1], "%d", club_pipes[i][0]);
            sprintf(argv[2], "%d", club_pipes[i][1]);

            if (execl(consts::EXE_CLUB, consts::EXE_CLUB, argv[0], argv[1], argv[2], NULL) == -1) {
                lg.error("Could not execute "s + argv[0]);
                return 1;
            }
        }
        else if (pid > 0) { // Parent process
            close(club_pipes[i][0]);
            write(club_pipes[i][1], buffer, consts::BUFF_SIZE);
            close(club_pipes[i][1]);
        }
    }

    // wait for club children
    for (int i = 0; i < clubs.size(); i++) {
        int status;
        wait(&status);
    }

    return EXIT_SUCCESS;
}