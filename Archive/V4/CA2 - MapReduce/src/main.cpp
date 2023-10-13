#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <error.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "../include/utils.h"
using namespace std;

vector<string> find_sub_directories(string path)
{
    int ls_pipe_fd[2];
    int grep_pipe_fd[2];
    if (pipe(ls_pipe_fd) == -1 || pipe(grep_pipe_fd) == -1)
    {
        perror("pipe error.");
    }
    int ls_pid = fork();
    if (ls_pid == 0) // child process
    {
        close(STDOUT_FILENO);
        close(ls_pipe_fd[0]);
        dup(ls_pipe_fd[1]); // make stdout same as ls_pipe_fd[1]
        execlp("ls", "ls", "-q", "--file-type", path.c_str(), NULL);
        close(ls_pipe_fd[1]);
        exit(1);
    }
    else if (ls_pid > 0) // parent process
    {
        close(ls_pipe_fd[1]);
        wait(NULL); // wait for ls
        int grep_pid = fork();
        if (grep_pid == 0) // child process
        {
            close(STDIN_FILENO);
            dup(ls_pipe_fd[0]); // read from ls

            close(STDOUT_FILENO);
            dup(grep_pipe_fd[1]);   // make stdout same as grep_pipe_fd[1]
            close(grep_pipe_fd[0]); // we don't need this
            execlp("grep", "grep", "/", NULL);
            exit(1);
        }
        else if (grep_pid > 0) // parent process
        {
            close(grep_pipe_fd[1]);
            char buffer[MAX_LEN] = {0};
            read(grep_pipe_fd[0], buffer, MAX_LEN);
            return split_buffer(buffer, '\n');
        }
    }
    return {};
}

void make_process(vector<string> positions, vector<string> sub_dirs, string clubs_folder)
{
    unlink(NAMED_PIPE); // remove if previously created
    if (mkfifo(NAMED_PIPE, S_IRWXU | 0666) != 0)
    {
        perror("first fifo not created properly.");
    }
    for (string pos : positions)
    {
        string read_end, write_end;
        int p_pip_fd[2];
        if (pipe(p_pip_fd) == -1)
        {
            perror("pipe error.");
        }
        int position_pid = fork(); // creating a new process for each position
        if (position_pid == 0)     // child process
        {
            read_end = to_string(p_pip_fd[0]);
            write_end = to_string(p_pip_fd[1]);
            execlp("./position.out", "./position.out", read_end.c_str(), write_end.c_str(), NULL);
            exit(EXIT_FAILURE);
        }
        else if (position_pid > 0) // parent process
        {
            string position_process_args(NAMED_PIPE);
            position_process_args += ("$" + pos);
            close(p_pip_fd[0]);
            write(p_pip_fd[1],
                  position_process_args.c_str(),
                  position_process_args.length());
            close(position_process_args[1]);
            for (string dir : sub_dirs)
            {
                int c_pip_fd[2];
                if (pipe(c_pip_fd) == -1)
                {
                    perror("pipe error.");
                }
                int dir_pid = fork();
                if (dir_pid == 0) // child process
                {
                    read_end = to_string(c_pip_fd[0]);
                    write_end = to_string(c_pip_fd[1]);
                    execlp("./country.out", "./country.out", read_end.c_str(), write_end.c_str());
                    exit(EXIT_FAILURE);
                }
                else if (dir_pid > 0) // parent process
                {
                    close(c_pip_fd[0]);
                    string to_send(NAMED_PIPE);
                    to_send += ("$" + clubs_folder + dir + '$' + pos);
                    write(c_pip_fd[1], to_send.c_str(), to_send.length());
                    waitpid(dir_pid, NULL, 0);
                }
            }
            // now process from the given informations from all countries
            waitpid(position_pid, NULL, 0);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        perror("usuage: ./ClubsAgeStats.out <clubs_folder>");
        return EXIT_FAILURE;
    }

    string clubs_folder = string(argv[1]);

    print_positions_from_file(clubs_folder);

    make_process(
        recieve_positions(),
        find_sub_directories(clubs_folder),
        clubs_folder);

    return EXIT_SUCCESS;
}