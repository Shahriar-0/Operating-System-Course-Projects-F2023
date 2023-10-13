#ifndef UTILS_HPP
#define UTILS_HPP

#include <filesystem>
#include <vector>

#include "logger.hpp"

using namespace std::string_literals;
namespace fs = std::filesystem;


int get_directory_folders(std::string path, std::vector<fs::path>& folders, Logger& lg) {
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

int get_directory_files(std::string path, std::vector<fs::path>& files, Logger& lg) {
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

#endif