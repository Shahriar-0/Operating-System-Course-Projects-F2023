#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "../include/utils.h"
using namespace std;

vector<string> split_buffer(const char *buffer, char delim)
{
    vector<string> vec;
    string s(buffer), temp;
    stringstream ss;
    ss << s;
    while (std::getline(ss, temp, delim))
    {
        vec.push_back(temp);
    }
    return vec;
}

void print_vector_elements(vector<string> vec)
{
    for (int i = 0; i < vec.size(); i++)
    {
        cout << vec[i];
        if (i != vec.size() - 1)
        {
            cout << "  -  ";
        }
    }
    cout << endl;
}

void print_positions_from_file(string clubs_folder)
{
    string positions_path = clubs_folder + "/positions.csv";
    ifstream file(positions_path);
    if (file.good())
    {
        string s;
        file >> s;
        cout << "All positions :" << endl;
        print_vector_elements(
            split_buffer(s.c_str(), ','));
    }
    else
    {
        perror("file not opened properly.");
        exit(EXIT_FAILURE);
    }
}

vector<string> recieve_positions()
{
    string s;
    cout << "Enter positions to get stats :" << endl;
    std::getline(cin, s);
    return split_buffer(s.c_str(), ' ');
}