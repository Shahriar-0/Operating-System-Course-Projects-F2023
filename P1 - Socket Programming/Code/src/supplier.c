#include "define.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        logError("Usage: ./supplier <port>");
        exit(EXIT_FAILURE);
    }

    Supplier supplier;
    supplier.tcpPort = atoi(argv[1]);
}