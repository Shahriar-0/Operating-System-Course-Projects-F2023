#include <unistd.h>

#include <iostream>

#include "company_process.hpp"
#include "console_logger.hpp"
#include "consts.hpp"
#include "file_console_logger.hpp"
#include "file_logger.hpp"
#include "logger.hpp"
#include "utils.hpp"

int main(int argc, char* argv[]) {
    Logger* log = new FileConsoleLogger(COMPANY_NAME);
    try {
        Company company(argv[1], log);
        sleep(1);
        company.run();
    } catch (std::runtime_error& e) {
        log->error(e.what());
        delete log;
        return 1;
    }

    return 0;
}
