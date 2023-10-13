#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

constexpr int MAX_LEN = 512;
constexpr char NAMED_PIPE[] = "position_country_pipe";
constexpr int SLEEP_TIME = 0.5 * 1000 * 1000; // 0.5 seconds

std::vector<std::string> split_buffer(const char *, char);

void print_vector_elements(std::vector<std::string>);

void print_positions_from_file(std::string);

std::vector<std::string> recieve_positions();

#endif