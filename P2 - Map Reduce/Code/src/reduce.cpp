#include "reduce.hpp"

Reduce::Reduce(const std::string& name, int id) : Node(name, id) {
    fifoPath_ = FIFO_PATH + name;
    mkfifo(fifoPath_.c_str(), 0777);
}

void Reduce::sendMessage(const std::string& cmd) {
    int fd = open(fifoPath_.c_str(), O_WRONLY);
    write(fd, cmd.c_str() + '\0', cmd.size() + 1);
    close(fd);
}

void Reduce::sendMessage(const std::string& cmd, const std::vector<std::string>& args) {
    std::string msg = encode(cmd, args) + '\0';
    int fd = open(fifoPath_.c_str(), O_WRONLY);
    write(fd, msg.c_str(), msg.size());
    close(fd);
}

std::string Reduce::receiveMessage() {
    std::vector<std::string> temp;
    return receiveMessage(temp);
}

std::string Reduce::receiveMessage(std::vector<std::string>& args) {
    int fd = open(fifoPath_.c_str(), O_RDONLY);
    char buf[BUFSIZE];
    read(fd, buf, BUFSIZE);
    close(fd);
    std::string cmd = decode(buf, args);
    return cmd;
}