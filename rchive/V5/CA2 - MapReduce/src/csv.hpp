#ifndef CSV_HPP
#define CSV_HPP

#include <string>
#include <vector>

class Csv {
public:
    using Table = std::vector<std::vector<std::string>>;

    Csv(std::string filename);

    int readfile();
    const Table& get() const;

private:
    Table table_;
    std::string filename_;
};

#endif
