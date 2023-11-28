#include "building_process.hpp"
#include "console_logger.hpp"
#include "consts.hpp"
#include "file_console_logger.hpp"
#include "file_logger.hpp"
#include "logger.hpp"

int main(int argc, char* argv[]) {
    Logger* log = new FileLogger(argv[2]);
    try {
        Building building(log, argv[2], argv[1]);
        building.run();
    } catch (std::runtime_error& e) {
        log->error(e.what());
        delete log;
        return 1;
    }

    return 0;
}
