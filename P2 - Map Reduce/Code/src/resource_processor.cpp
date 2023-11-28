#include "console_logger.hpp"
#include "consts.hpp"
#include "file_console_logger.hpp"
#include "file_logger.hpp"
#include "logger.hpp"
#include "resource_process.hpp"

int main(int argc, char* argv[]) {
    Logger* log = new FileLogger(std::string(argv[2]) + "_" + argv[3]);
    try {
        Resource resource(log, argv[3], argv[1]);
        resource.run();
    } catch (std::runtime_error* e) {
        log->error(e->what());
        delete e;
        return 1;
    }

    return 0;
}
