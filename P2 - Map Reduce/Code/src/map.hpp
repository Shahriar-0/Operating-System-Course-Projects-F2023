#ifndef MAP_HPP_INCLUDE
#define MAP_HPP_INCLUDE

#include "consts.hpp"
#include "utils.hpp"
#include "node.hpp"

class Map : public Node {
public:
    Map(const std::string& name, int writefd, int readfd, int id);
    virtual void sendMessage(const std::string& cmd);
    virtual void sendMessage(const std::string& cmd, const std::vector<std::string>& args);
    virtual std::string receiveMessage();
    virtual std::string receiveMessage(std::vector<std::string>& args);

private:
    int writefd_;
    int readfd_;
};

#endif