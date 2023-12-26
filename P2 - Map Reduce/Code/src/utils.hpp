#ifndef UTILS_HPP_INCLUDE
#define UTILS_HPP_INCLUDE

#include <random>
#include "consts.hpp"

std::vector<std::string> GetFilesOfDirectory(const char* path);
char** vectorToCharArray(const std::vector<std::string>& argsIn);
std::vector<std::string> charArrayToVector(char** argsIn, int run, int end);
void setupUnnamedPipes(int* fds1, int* fds2);
void writefd(int fd, const std::string& cmd, const std::vector<std::string>& args);
std::string readfd(int fd, std::vector<std::string>& args);
std::string encode(const std::string& cmd, const std::vector<std::string>& args);
std::string decode(const std::string& msg, std::vector<std::string>& args);
void trimLeft(std::string& str);
void trimRight(std::string& str);
void trim(std::string& str);
std::vector<std::string> split(const std::string& str, char delim);
std::vector<std::string> split(const std::string& str, const std::string& delim);
void emptyLog();
std::string prompt(std::string msg);
std::string vectorToString(const std::vector<std::string>& args);
void printRes(const std::string& msg, const std::string& res);

#endif