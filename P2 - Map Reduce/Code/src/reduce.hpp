#ifndef REDUCE_HPP_INCLUDE
#define REDUCE_HPP_INCLUDE

#include "consts.hpp"
#include "utils.hpp"
#include "node.hpp"

class Reduce : public Node {
public:
    Reduce(const std::string& name, int id);
    virtual void sendMessage(const std::string& cmd);
    virtual void sendMessage(const std::string& cmd, const std::vector<std::string>& args);
    virtual std::string receiveMessage();
    virtual std::string receiveMessage(std::vector<std::string>& args);

private:
    std::string fifoPath_;
};

#endif