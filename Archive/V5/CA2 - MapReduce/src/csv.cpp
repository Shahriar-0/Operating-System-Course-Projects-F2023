#include "csv.hpp"

#include <fstream>

#include "strutils.hpp"

Csv::Csv(std::string filename)
    : filename_(std::move(filename)) {}

int Csv::readfile() {
    std::ifstream file(filename_);
    if (!file.is_open()) return 1;

    table_.clear();

    std::string line;
    while (std::getline(file, line)) {
        auto splt = strutils::split(line, ',');
        for (std::string& part : splt) {
            strutils::trim(part);
        }
        table_.push_back(std::move(splt));
    }

    return 0;
}

const Csv::Table& Csv::get() const {
    return table_;
}
