#ifndef COMMUNICATION_HPP_INCLUDE
#define COMMUNICATION_HPP_INCLUDE

#include "consts.hpp"
#include "utils.hpp"

class Node {
public:
    Node(const std::string& name, int id) : name_(name), id_(id) {}
    virtual void sendMessage(const std::string& cmd) = 0;
    virtual void sendMessage(const std::string& msg, const std::vector<std::string>& args) = 0;
    virtual std::string receiveMessage() = 0;
    virtual std::string receiveMessage(std::vector<std::string>& args) = 0;
    std::string getName() { return name_; }

protected:
    std::string name_;
    int id_;
};

#endif
