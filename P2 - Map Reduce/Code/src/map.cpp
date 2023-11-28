#include "map.hpp"

Map::Map(const std::string& name, int writefd, int readfd, int id) : Node(name, id) {
    writefd_ = writefd;
    readfd_ = readfd;
}

void Map::sendMessage(const std::string& cmd) {
    write(writefd_, cmd.c_str() + '\0', cmd.size() + 1);
}

void Map::sendMessage(const std::string& cmd, const std::vector<std::string>& args) {
    std::string msg = encode(cmd, args) + '\0';
    write(writefd_, msg.c_str(), msg.size());
}

std::string Map::receiveMessage() {
    std::vector<std::string> temp;
    return receiveMessage(temp);
}

std::string Map::receiveMessage(std::vector<std::string>& args) {
    char buf[BUFSIZE];
    read(readfd_, buf, BUFSIZE);
    return decode(std::string(buf), args);
}