#include "utils.hpp"

#include "consts.hpp"

std::vector<std::string> GetFilesOfDirectory(const char* path) {
    std::vector<std::string> names;

    std::filesystem::directory_iterator dirIter(path);
    for (const auto& entry : dirIter)
        if (entry.is_directory()) names.push_back(entry.path().filename().string());

    return names;
}

std::vector<std::string> charArrayToVector(char** args, int run, int end) {
    std::vector<std::string> re;
    for (int i = run; i <= end; i++) re.push_back(args[i]);
    return re;
}

char** vectorToCharArray(const std::vector<std::string>& args) {
    char** generated = (char**)malloc((args.size() + 1) * sizeof(char*));

    for (long unsigned int i = 0; i < args.size(); i++) {
        generated[i] = (char*)malloc((args[i].size() + 1) * sizeof(char));
        std::copy(args[i].begin(), args[i].end(), generated[i]);
        generated[i][args[i].size()] = '\0';
    }
    generated[args.size()] = nullptr;

    return generated;
}

void setupUnnamedPipes(int* fds1, int* fds2) {
    close(fds2[0]);
    dup2(fds1[0], STDIN_FILENO);
    close(fds1[0]);
    close(fds1[1]);
    dup2(fds2[1], STDOUT_FILENO);
    close(fds2[1]);
}

void writefd(int fd, const std::string& cmd, const std::vector<std::string>& args) {
    std::string msg = encode(cmd, args);
    write(fd, (msg + '\0').c_str(), msg.size() + 1);
}

std::string readfd(int fd, std::vector<std::string>& args) {
    char buf[BUFSIZE];
    read(fd, buf, BUFSIZE);
    return decode(std::string(buf), args);
}

void trimLeft(std::string& str) {
    str.erase(str.begin(), std::find_if(str.begin(), str.end(),
                                        [](unsigned char ch) { return !std::isspace(ch); }));
}

void trimRight(std::string& str) {
    str.erase(
        std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); })
            .base(),
        str.end());
}

void trim(std::string& str) {
    trimLeft(str);
    trimRight(str);
}

std::vector<std::string> split(const std::string& str, char delim) {
    std::istringstream sstr(str);
    std::string item;
    std::vector<std::string> result;
    while (std::getline(sstr, item, delim)) {
        result.push_back(std::move(item));
    }
    return result;
}

std::string encode(const std::string& cmd, const std::vector<std::string>& args) {
    std::string msg = cmd;
    for (const std::string& arg : args) msg += " " + arg;
    return msg;
}

std::string decode(const std::string& msg, std::vector<std::string>& args) {
    std::stringstream ss(msg);
    args.clear();
    std::string substr;
    while (getline(ss, substr, ' ')) args.push_back(substr);
    std::string cmd = args[0];
    args.erase(args.begin());
    return cmd;
}

std::vector<std::string> split(const std::string& str, const std::string& delim) {
    std::string::size_type runPos = 0;
    std::string::size_type endPos;

    std::vector<std::string> result;
    while ((endPos = str.find(delim, runPos)) != std::string::npos) {
        result.push_back(str.substr(runPos, endPos - runPos));
        runPos = endPos + delim.size();
    }
    result.push_back(str.substr(runPos));

    return result;
}

void emptyLog() {
    std::ofstream ofs;
    ofs.open(LOG_PATH, std::ofstream::out | std::ofstream::trunc);
    ofs.close();
}

ResourceUsage::ResourceUsage(std::string CSVPath) {
    CSVPath_ = CSVPath;
    loadCSV();
}

void ResourceUsage::loadCSV() {
    std::ifstream CSVfile(CSVPath_);
    std::string line;
    std::getline(CSVfile, line);
    while (std::getline(CSVfile, line)) {
        std::vector<std::string> tempParam = split(line, ",");
        trim(tempParam[0]);
        trim(tempParam[1]);
        trim(tempParam[2]);

        ResourceUsageEntry* newEntry = new ResourceUsageEntry;
        initResourceUsage(newEntry, tempParam);

        entries_.push_back(newEntry);
    }
}

void Bill::loadCSV() {
    std::ifstream CSVfile(CSVPath_);
    std::string line;
    std::getline(CSVfile, line);
    while (std::getline(CSVfile, line)) {
        std::vector<std::string> tempParam = split(line, ",");
        trim(tempParam[0]);
        trim(tempParam[1]);
        trim(tempParam[2]);
        trim(tempParam[3]);
        trim(tempParam[4]);

        BillEntry* newEntry = new BillEntry();
        initBillEntry(newEntry, tempParam);

        entries_.push_back(newEntry);
    }
}

void Bill::initBillEntry(BillEntry*& newEntry, std::vector<std::string>& tempParam) {
    newEntry->m = atoi(tempParam[1].c_str());
    newEntry->water = atoi(tempParam[2].c_str());
    newEntry->gas = atoi(tempParam[3].c_str());
    newEntry->electricity = atoi(tempParam[4].c_str());
}

void ResourceUsage::initResourceUsage(ResourceUsageEntry*& newEntry,
                                      std::vector<std::string>& tempParam) {
    newEntry->m = atoi(tempParam[1].c_str());
    newEntry->d = atoi(tempParam[2].c_str());
    assignHours(newEntry, tempParam);
}

std::string vectorToString(const std::vector<std::string>& args) {
    std::string re;
    for (const auto& arg : args) re += arg + " ";
    return re;
}

void ResourceUsage::assignHours(ResourceUsageEntry* newEntry, std::vector<std::string>& tempParam) {
    for (long unsigned int i = 0; i < HOUR_COUNT; i++)
        newEntry->hoursUsage[i] = atoi(tempParam[i + 3].c_str());
}

int ResourceUsage::mean(int m) {
    int sum = 0;
    for (long unsigned int i = 0; i < entries_.size(); i++)
        if (entries_[i]->m == m)
            for (long unsigned int j = 0; j < HOUR_COUNT; j++) sum += entries_[i]->hoursUsage[j];

    return sum / 30;
}

int ResourceUsage::diff(int month) {
    int average = mean(month) / HOUR_COUNT;
    std::vector<int> max = maxUsage(month);
    int totalMax = totalUsage(month, max[0]) / 30;
    return totalMax - average;
}

std::string prompt(std::string msg) {
    std::cout << MAGENTA << msg << RESET;
    std::string re;
    std::getline(std::cin, re);
    return re;
}

void printRes(const std::string& res) { std::cout << "---------------> " << res << std::endl; }

std::vector<int> ResourceUsage::maxUsage(int m) {
    int totalUsage[HOUR_COUNT];
    for (long unsigned int i = 0; i < HOUR_COUNT; i++) {
        totalUsage[i] = 0;
        for (long unsigned int j = 0; j < entries_.size(); j++)
            if (entries_[j]->m == m) totalUsage[i] += entries_[j]->hoursUsage[i];
    }

    std::vector<int> maxUsagesIndex;
    int maxUsage = 0;

    for (long unsigned int i = 0; i < HOUR_COUNT; i++) {
        if (maxUsage < totalUsage[i]) {
            maxUsage = totalUsage[i];
            maxUsagesIndex.clear();
            maxUsagesIndex.push_back(i);
        } else if (maxUsage == totalUsage[i]) {
            maxUsagesIndex.push_back(i);
        }
    }
    return maxUsagesIndex;
}

int ResourceUsage::totalUsage(int m, int hour) {
    int sum = 0;
    for (long unsigned int i = 0; i < entries_.size(); i++)
        if (entries_[i]->m == m) sum += entries_[i]->hoursUsage[hour];

    return sum;
}

int ResourceUsage::water(int m, int p) {
    int re = 0;
    std::vector<int> maxHours = maxUsage(m);

    for (const auto& record : entries_)
        if (record->m == m)
            for (long unsigned int j = 0; j < HOUR_COUNT; j++)
                re += (std::find(maxHours.begin(), maxHours.end(), j) != maxHours.end())
                          ? record->hoursUsage[j] * 1.25
                          : record->hoursUsage[j];

    return re * p;
}

int ResourceUsage::gas(int m, int p) {
    int re = 0;
    int totalUsage = mean(m) * 30;
    re = totalUsage * p;
    return re;
}

int ResourceUsage::electricity(int m, int p) {
    int re = 0;
    std::vector<int> maxHours = maxUsage(m);

    for (const auto& record : entries_) 
        if (record->m == m) 
            for (long unsigned int j = 0; j < HOUR_COUNT; j++) {
                int high = record->hoursUsage[j];
                re += (std::find(maxHours.begin(), maxHours.end(), j) != maxHours.end())
                          ? high * 1.25
                          : (high < mean(m) / HOUR_COUNT) ? high * 0.75
                                                          : high;
            }

    return re * p;
}

std::vector<int> Bill::calcParam(int m) {
    for (long unsigned int i = 0; i < entries_.size(); i++)
        if (entries_[i]->m == m)
            return {entries_[i]->water, entries_[i]->gas, entries_[i]->electricity};

    throw new std::runtime_error("No Record Found");
}

Bill::Bill(std::string CSVPath) {
    CSVPath_ = CSVPath;
    loadCSV();
}
