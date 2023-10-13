#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <regex>
#include <vector>

#include "consts.hpp"
#include "csv.hpp"
#include "logger.hpp"
#include "strutils.hpp"

using namespace std::string_literals;
namespace fs = std::filesystem;

Logger lg("main");

int get_directory_folders(std::string path, std::vector<fs::path>& folders) {
    if (fs::exists(path) && fs::is_directory(path)) {
        fs::path directory_path = path;
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            if (entry.is_directory()) {
                folders.push_back(entry.path());
            }
        }
    }
    else {
        lg.error("Could not open directory: "s + path);
        return 1;
    }
    return 0;
}

int createPosList(std::vector<std::string>& pos_list, std::string pos_path) {
    Csv csv(pos_path);
    csv.readfile();
    auto tbl = csv.get();

    if (tbl.size() < 1) {
        lg.error("Positions file empty.");
        return 1;
    }
    pos_list = tbl[0];
    return 0;
}

int getPosWanted(std::vector<std::string>& pos_wanted, const std::vector<std::string>& pos_list) {
    // print positions
    std::cout << "ALL Positions:\n";
    for (auto& pos : pos_list) {
        std::cout << pos << " ";
    }

    // get positions wanted from user space separated
    std::string input;
    std::cout << "\nEnter positions wanted (space separated): ";
    std::getline(std::cin, input);

    // split input into vector
    std::regex re("\\s+");
    std::sregex_token_iterator first{input.begin(), input.end(), re, -1}, last;
    std::vector<std::string> pos_wanted_vec{first, last};

    // check if positions are valid
    for (auto& pos : pos_wanted_vec) {
        if (std::find(pos_list.begin(), pos_list.end(), pos) == pos_list.end()) {
            lg.error("Invalid position: "s + pos);
            return 1;
        }
    }

    pos_wanted = pos_wanted_vec;
    return 0;
}

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: "
                  << "ClubsAgeStats.out"
                  << " <clubs folder>\n";
        return EXIT_FAILURE;
    }

    std::vector<std::string> pos_list, pos_wanted;
    std::vector<fs::path> countries;

    if (fs::exists(consts::PIPES_PATH)) {
        fs::remove_all(consts::PIPES_PATH);
    }

    if (mkdir(consts::PIPES_PATH, 0777) == -1) {
        lg.error("Could not create pipes directory");
        return EXIT_FAILURE;
    }

    if (createPosList(pos_list, consts::POS_PATH))
        return EXIT_FAILURE;

    if (getPosWanted(pos_wanted, pos_list))
        return EXIT_FAILURE;

    if (get_directory_folders(argv[1], countries))
        return EXIT_FAILURE;

    // create unnamed pipes for country
    int country_pipes[countries.size()][2];
    for (int i = 0; i < countries.size(); i++) {
        if (pipe(country_pipes[i]) == -1) {
            lg.error("Could not create country pipe");
            return EXIT_FAILURE;
        }
    }

    // create process for each counrty
    for (int i = 0; i < countries.size(); i++) {
        int pid = fork();

        if (pid < 0) {
            lg.error("Could not create child process for position: "s + countries[i].c_str());
        }
        else if (pid == 0) { // Child process
            char argv[3][256];
            sprintf(argv[0], "%s", countries[i].c_str());
            sprintf(argv[1], "%d", country_pipes[i][0]);
            sprintf(argv[2], "%d", country_pipes[i][1]);

            if (execl(consts::EXE_COUNTRY, consts::EXE_COUNTRY, argv[0], argv[1], argv[2], NULL) == -1) {
                lg.error("Could not execute "s + argv[0]);
                return 1;
            }
        }
        else if (pid > 0) { // Parent process
            std::string msg = strutils::join(pos_wanted, ",");
            close(country_pipes[i][0]);
            write(country_pipes[i][1], msg.c_str(), msg.size());
            close(country_pipes[i][1]);
        }
    }

    sleep(1);

    // create process for each position
    for (int i = 0; i < pos_wanted.size(); i++) {
        int pid = fork();

        if (pid < 0) {
            lg.error("Could not create child process for position: "s + pos_wanted[i]);
        }
        else if (pid == 0) { // Child process
            char argv[256];
            sprintf(argv, "%s", pos_wanted[i].c_str());
            if (execl(consts::EXE_POS, consts::EXE_POS, argv, NULL) == -1) {
                lg.error("Could not execute "s + argv[0]);
                return 1;
            }
        }
    }

    // wait for country children
    for (int i = 0; i < countries.size(); i++) {
        int status;
        wait(&status);
    }

    // wait for position children
    for (int i = 0; i < pos_wanted.size(); i++) {
        int status;
        wait(&status);
    }

    return EXIT_SUCCESS;
}
