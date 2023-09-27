#include "strutils.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace strutils {

void trimLeft(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
                  return !std::isspace(ch);
              }));
}

void trimRight(std::string& str) {
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
                  return !std::isspace(ch);
              }).base(),
              str.end());
}

void trim(std::string& str) {
    trimLeft(str);
    trimRight(str);
}

std::vector<std::string> split(const std::string& str, char delim) {
    std::istringstream sstr(str);
    std::string item;
    std::vector<std::string> result;
    while (std::getline(sstr, item, delim)) {
        result.push_back(std::move(item));
    }
    return result;
}

std::vector<std::string> split(const std::string& str, const std::string& delim) {
    std::string::size_type startPos = 0;
    std::string::size_type endPos;

    std::vector<std::string> result;
    while ((endPos = str.find(delim, startPos)) != std::string::npos) {
        result.push_back(str.substr(startPos, endPos - startPos));
        startPos = endPos + delim.size();
    }
    result.push_back(str.substr(startPos));

    return result;
}

std::string join(const std::vector<std::string>& vec, std::string delim) {
    std::string result;
    for (auto& item : vec) {
        result += item + delim;
    }
    result.pop_back();
    return result;
}

} // namespace strutils

