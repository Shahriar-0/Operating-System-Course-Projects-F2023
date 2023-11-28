#include "console_logger.hpp"
#include "consts.hpp"
#include "file_console_logger.hpp"
#include "file_logger.hpp"
#include "financial_process.hpp"
#include "logger.hpp"
#include "utils.hpp"

int main(int argc, char* argv[]) {
    Logger* log = new FileLogger(std::string(argv[1]));
    try {
        Financial financial(log, argv[1], charArrayToVector(argv, 2, argc - 1));
        financial.run();
    } catch (std::runtime_error*& e) {
        log->error(e->what());
        delete e;
        return 1;
    }

    return 0;
}
