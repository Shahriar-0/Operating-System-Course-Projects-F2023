#ifndef CONSTS_HPP_INCLUDE
#define CONSTS_HPP_INCLUDE

#include <bits/stdc++.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#define EXE_FINANCIAL "financial.out"
#define EXE_BUILDING "building.out"
#define EXE_RESOURCE "resource.out"

#define FINANCIAL_NAME "FINANCIAL"
#define COMPANY_NAME "COMPANY"

#define DEFAULT_ID 180
#define BUFSIZE 1024

#define FIFO_PATH "../fifos/"
#define LOG_PATH "../logs/"
#define LOG_FILE "../logs/main.log"

#define MSG_OK "OK"
#define MSG_NOT_OK "Problem occurred"
#define MSG_CLOSE "close"
#define MSG_HELP "help"
#define MSG_REPORT "report"
#define MSG_REPORT_FOR_DIFFERENCE "difference"
#define MSG_REPORT_FOR_MAX "max"
#define MSG_RESPONSE "response"
#define MSG_REPORT_FOR_TOTAL "total"
#define MSG_REPORT_FOR_MEAN "mean"
#define MSG_REPORT_BILL "bill"
#define MSG_REPORT_DATA "data"
#define MSG_BUILDINGS "buildings"
#define MSG_EXIT "exit"

#define WATER "Water"
#define GAS "Gas"
#define ELECTRICITY "Electricity"

#define RESOURCE_COUNT 3
#define MONTH_COUNT 12
#define HOUR_COUNT 6

#define billCSVPath "../buildings/bills.csv"

#define RESET "\033[0m"
#define RED "\033[35m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

struct ResourceUsageEntry {
    int month, d, hoursUsage[HOUR_COUNT];
};

struct BillEntry {
    int month, water, gas, electricity;
};

class ResourceUsage {
public:
    ResourceUsage(std::string CSVPath);
    int mean(int month);
    int diff(int month);
    int water(int month, int p);
    int gas(int month, int p);
    int electricity(int month, int p);
    int totalUsage(int month, int hour);
    std::vector<int> maxUsage(int month);

private:
    void loadCSV();
    void initResourceUsage(ResourceUsageEntry*& newEntry, std::vector<std::string>& tempParam);
    void assignHours(ResourceUsageEntry* newEntry, std::vector<std::string>& tempParam);

    std::string CSVPath_;
    std::vector<ResourceUsageEntry*> entries_;
};

class Bill {
public:
    Bill(std::string CSVPath);
    std::vector<int> calc(int month);

private:
    void loadCSV();
    void initBillEntry(BillEntry*& newEntry, std::vector<std::string>& tempParam);

    std::string CSVPath_;
    std::vector<BillEntry*> entries_;
};

#endif
