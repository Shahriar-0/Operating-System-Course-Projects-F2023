#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <error.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include "../include/utils.h"
using namespace std;

string get_related_file_data(string csv_path, string player_position)
{
    ifstream file(csv_path);
    string matched_positions, temp;
    if (file.good())
    {
        while (getline(file, temp))
        {
            vector<string> players_info = split_buffer(temp.c_str(), ',');
            if (players_info[1] == player_position)
            {
                matched_positions += (temp + '\n');
            }
        }
        return matched_positions;
    }
    else
    {
        perror("bad file");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("bad club args.");
        exit(EXIT_FAILURE);
    }
    int read_fd = atoi(argv[1]);
    int write_fd = atoi(argv[2]);
    char buffer[MAX_LEN] = {0};
    read(read_fd, buffer, MAX_LEN); // read csv path
    close(read_fd);
    vector<string> args = split_buffer(buffer, '$');
    string to_write = get_related_file_data(args[0], args[1]);
    if (!to_write.empty())
    {
        write(write_fd, to_write.c_str(), to_write.length());
    }
    close(write_fd);
    exit(EXIT_SUCCESS);
}