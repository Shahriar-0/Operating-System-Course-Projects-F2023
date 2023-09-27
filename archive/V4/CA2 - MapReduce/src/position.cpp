#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <error.h>
#include <fcntl.h>
#include <algorithm>
#include <numeric>
#include <iomanip>
#include "../include/utils.h"
using namespace std;

int min_age(vector<int> intAges)
{
    return *min_element(begin(intAges), end(intAges));
}

int max_age(vector<int> intAges)
{
    return *max_element(begin(intAges), end(intAges));
}

float avg_age(vector<int> intAges)
{
    int sum = accumulate(begin(intAges), end(intAges), 0);
    return static_cast<float>(sum) / static_cast<float>(intAges.size());
}

void show_report(string pos, string args)
{
    vector<string> ages_str = split_buffer(args.c_str(), '$');
    vector<int> ages_int(ages_str.size());
    for (int i = 0; i < ages_int.size(); i++)
    {
        ages_int[i] = stoi(ages_str[i]);
    }
    cout << pos << " min age : " << (args.empty() ? 0 : min_age(ages_int)) << endl
         << pos << " max age : " << (args.empty() ? 0 : max_age(ages_int)) << endl
         << pos << " avg age : " << std::setprecision(3) << (args.empty() ? 0 : avg_age(ages_int)) << endl
         << pos << " count : " << ages_int.size() << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("bad position args.");
        exit(EXIT_FAILURE);
    }
    int read_fd = atoi(argv[1]);
    int write_fd = atoi(argv[2]);
    close(write_fd);
    char buffer[MAX_LEN] = {0};
    if (read(read_fd, buffer, MAX_LEN) == -1) // read sent buffer
    {
        perror("nothing received.");
        exit(EXIT_FAILURE);
    }
    close(read_fd);
    vector<string> args = split_buffer(buffer, '$');
    int fd = open(args[0].c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        perror("error in openning file.");
    }
    usleep(SLEEP_TIME);
    // wait for writer to write to the pipe (country) -> 0.5 s
    // writter connected here
    string ages = "";
    char buf[MAX_LEN];
    memset(buf, 0, MAX_LEN);
    memset(buf, 0, MAX_LEN);
    while (read(fd, buf, MAX_LEN) > 0)
    {
        ages += buf;
        memset(buf, 0, MAX_LEN);
    }

    show_report(args[1].c_str(), ages);
    exit(EXIT_SUCCESS);
}