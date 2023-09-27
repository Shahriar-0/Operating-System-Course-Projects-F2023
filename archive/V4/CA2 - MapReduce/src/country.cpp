#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <error.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include "../include/utils.h"
using namespace std;

string concat_args(vector<string> sepAges)
{
    string all_ages;
    if (!sepAges.empty())
    {
        for (string age : sepAges)
        {
            all_ages += age;
            all_ages += '$';
        }
    }
    return all_ages;
}

void find_clubs(vector<string> &clubs, const char *path)
{
    int ls_pipe[2];
    if (pipe(ls_pipe) == -1)
    {
        perror("pipe error.");
    }
    int pid = fork();
    if (pid == 0) // child process
    {
        close(1);
        dup(ls_pipe[1]);
        close(ls_pipe[0]);
        execlp("ls", "ls", "-q", path, NULL);
        close(ls_pipe[1]);
        exit(EXIT_SUCCESS);
    }
    else if (pid > 0) // parent process
    {
        wait(NULL);
        close(ls_pipe[1]);
        char buf[MAX_LEN];
        memset(buf, 0, MAX_LEN);
        read(ls_pipe[0], buf, MAX_LEN);
        clubs = split_buffer(buf, '\n');
    }
}

vector<string> separate_matching_from_csv(const char *playerPos,
                                          const char *fileContent)
{
    vector<string> res;
    vector<string> file_lines =
        split_buffer(fileContent, '\n');
    for (string line : file_lines)
    {
        vector<string> line_content =
            split_buffer(line.c_str(), ',');
        if (line_content[1] ==
            string(playerPos))
        {
            res.push_back(
                line_content[2]);
        }
    }
    return res;
}

void position_and_country_communicaton(const char *pipeName, vector<string> clubs,
                                       const char *playerPos, const char *countryPath)
{
    int position_country_link = open(pipeName, O_WRONLY);
    string allAges;
    if (position_country_link < 0)
    {
        perror("while openning write fifo");
    }
    for (string club : clubs)
    {
        int countrClubPipe[2];
        if (pipe(countrClubPipe) != 0)
        {
            perror("pipe error");
        }
        int clubf = fork(); // making a process for each club
        if (clubf == 0)
        {
            const char *read_fd = to_string(countrClubPipe[0]).c_str();
            const char *write_fd = to_string(countrClubPipe[1]).c_str();
            execlp("./club.out", "./club.out", read_fd,
                   write_fd, NULL); // passing pipe FDs
            exit(EXIT_FAILURE);
        }
        else
        {
            string pipeSend;
            string csvPath = countryPath + club;
            pipeSend += csvPath + '$';
            pipeSend += playerPos;
            write(countrClubPipe[1], pipeSend.c_str(),
                  pipeSend.length());
            close(countrClubPipe[1]); // first write the path it has to read then close
            waitpid(clubf, NULL, 0);
            char csvContent[MAX_LEN];
            memset(csvContent, 0, MAX_LEN);
            read(countrClubPipe[0], csvContent, MAX_LEN);
            close(countrClubPipe[0]);
            allAges += concat_args(separate_matching_from_csv(
                playerPos,
                csvContent));
        }
    }
    if (!allAges.empty())
    {
        write(position_country_link, allAges.c_str(),
              allAges.length());
    }
    close(position_country_link);
    exit(EXIT_SUCCESS);
    // + finding corresponding position
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        perror("bad country args");
        exit(EXIT_FAILURE);
    }
    int read_fd = atoi(argv[1]);
    int write_fd = atoi(argv[2]);
    close(write_fd);
    char readArgs[MAX_LEN];
    memset(readArgs, 0, MAX_LEN);
    read(read_fd, readArgs, MAX_LEN);
    close(read_fd);
    vector<string> delArgs =
        split_buffer(readArgs, '$');
    vector<string> clubs;
    find_clubs(clubs, delArgs[1].c_str());
    position_and_country_communicaton(delArgs[0].c_str(), clubs,
                                      delArgs[2].c_str(), delArgs[1].c_str());
    exit(EXIT_SUCCESS);
}