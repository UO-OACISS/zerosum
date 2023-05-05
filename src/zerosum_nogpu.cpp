/**********************************************************
"Hello World"-type program to test different srun layouts.

Written by Tom Papatheodore
**********************************************************/

#include "zerosum.h"
#include <cstdio>
#include <regex>
#include <iostream>
#include <vector>
#include <string>
#include "perfstubs.h"

int ZeroSum::getgpu(const int rank, const int section, const char * name) {
    printf("MPI %03d - SEC %d - Node %s\n", rank, section, name);
    return 0;
}

std::string parseDiscreteValues(std::string inputString) {
    std::vector<int> result;

    // Split the input string by comma
    size_t pos = 0;
    std::string token;
    while ((pos = inputString.find(',')) != std::string::npos) {
        token = inputString.substr(0, pos);

        // Split each token by hyphen
        size_t hyphenPos = token.find('-');
        if (hyphenPos != std::string::npos) {
            int start = std::stoi(token.substr(0, hyphenPos));
            int end = std::stoi(token.substr(hyphenPos + 1));
            for (int i = start; i <= end; ++i) {
                result.push_back(i);
            }
        } else {
            result.push_back(std::stoi(token));
        }

        // Trim the processed token from the input string
        inputString.erase(0, (pos + 1));
    }

    // Process the last token after the last comma
    if (!inputString.empty()) {
        size_t hyphenPos = inputString.find('-');
        if (hyphenPos != std::string::npos) {
            int start = std::stoi(inputString.substr(0, hyphenPos));
            int end = std::stoi(inputString.substr(hyphenPos + 1));
            for (int i = start; i <= end; ++i) {
                result.push_back(i);
            }
        } else {
            result.push_back(std::stoi(inputString));
        }
    }

    std::stringstream ss;
    bool comma = false;
    for (int value : result) {
        if (comma) ss << ",";
        ss << value;
        comma = true;
    }
    std::string tmpstr{ss.str()};
    return tmpstr;
}

void ZeroSum::getProcStatus(const int section) {
    FILE *f = fopen("/proc/self/status", "r");
    if (!f) { return; }
    const std::string allowed("Cpus_allowed_list");
    std::string allowed_list;
    char line[4097] = {0};
    while ( fgets( line, 4096, f)) {
        std::string tmp(line);
        if (!tmp.compare(0,allowed.size(),allowed)) {
            const std::regex separator(":");
            std::sregex_token_iterator token(tmp.begin(),
                    tmp.end(), separator, -1);
            std::sregex_token_iterator end;
            std::string name = *token++;
            if (token != end) {
                allowed_list = *token;
                allowed_list.erase(
                    std::remove_if(
                        allowed_list.begin(), allowed_list.end(),
                        ::isspace),
                    allowed_list.end());
            }
        }
    }
    fclose(f);
    std::string discrete = parseDiscreteValues(allowed_list);
    printf("MPI %03d - SEC %d - Node %s - CPUs allowed: [%s]\n",
        rank, section, name, discrete.c_str());
    PERFSTUBS_METADATA("CPUs allowed", discrete.c_str());
    return;
}
