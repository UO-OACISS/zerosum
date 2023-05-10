#ifndef _GNU_SOURCE
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif
#define _XOPEN_SOURCE 700
#include <sched.h>
#include "utils.h"
#include "perfstubs.h"
#include <regex>
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>

namespace zerosum {

std::vector<uint32_t> parseDiscreteValues(std::string inputString) {
    std::vector<uint32_t> result;

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

    return result;
}

std::string getCpusAllowed(const char * filename) {
    //std::cout << std::endl << filename << std::endl;
    FILE *f = fopen(filename, "r");
    std::string allowed_string{""};
    if (!f) { return(allowed_string); }
    const std::string allowed("Cpus_allowed_list");
    char line[4097] = {0};
    while ( fgets( line, 4096, f)) {
        std::string tmp(line);
        //std::cout << tmp << std::flush;
        if (!tmp.compare(0,allowed.size(),allowed)) {
            const std::regex separator(":");
            std::sregex_token_iterator token(tmp.begin(),
                    tmp.end(), separator, -1);
            std::sregex_token_iterator end;
            std::string name = *token++;
            if (token != end) {
                allowed_string = *token;
                allowed_string.erase(
                    std::remove_if(
                        allowed_string.begin(), allowed_string.end(),
                        ::isspace),
                    allowed_string.end());
            }
        }
    }
    fclose(f);
    return(allowed_string);
}

std::map<std::string, std::string> getThreadStat(const char * filename) {
    std::map<std::string, std::string> fields;
    //std::cout << std::endl << filename << std::endl;
    FILE *f = fopen(filename, "r");
    std::string allowed_string{""};
    if (!f) { return fields; }
    char line[4097] = {0};
    std::vector<std::string> v;
    // insert a dummy so our indexing matches the documentation for the /proc/self/stat values.
    v.push_back("");
    while ( fgets( line, 4096, f)) {
        std::string tmp;
        std::stringstream ss(line);
        //std::cout << filename << " : " << line << std::endl;
        while (getline(ss, tmp, ' ')) {
            // store token string in the vector
            v.push_back(tmp);
        }
    }
    // parsing the fields as defined by https://man7.org/linux/man-pages/man5/proc.5.html
    fields.insert(std::pair("state", v[3]));
    fields.insert(std::pair("minflt", v[10]));
    fields.insert(std::pair("majflt", v[12]));
    fields.insert(std::pair("utime", v[14]));
    fields.insert(std::pair("stime", v[15]));
    fields.insert(std::pair("nswap", v[36]));
    fields.insert(std::pair("processor", v[39]));
    fclose(f);
    return fields;
}

void getThreadStatus(const char * filename, std::map<std::string, std::string>& fields) {
    //std::cout << std::endl << filename << std::endl;
    FILE *f = fopen(filename, "r");
    std::string tmpstr{""};
    if (!f) { return; }
    const std::string ctx("voluntary_ctxt_switches");
    const std::string nvctx("nonvoluntary_ctxt_switches");
    char line[4097] = {0};
    while ( fgets( line, 4096, f)) {
        std::string tmp(line);
        if (!tmp.compare(0,ctx.size(),ctx)) {
            const std::regex separator(":");
            std::sregex_token_iterator token(tmp.begin(),
                    tmp.end(), separator, -1);
            std::sregex_token_iterator end;
            std::string name = *token++;
            if (token != end) {
                tmpstr = *token;
                tmpstr.erase(
                    std::remove_if(
                        tmpstr.begin(), tmpstr.end(),
                        ::isspace),
                    tmpstr.end());
            }
            fields.insert(std::pair(ctx, tmpstr));
        }
        if (!tmp.compare(0,nvctx.size(),nvctx)) {
            const std::regex separator(":");
            std::sregex_token_iterator token(tmp.begin(),
                    tmp.end(), separator, -1);
            std::sregex_token_iterator end;
            std::string name = *token++;
            if (token != end) {
                tmpstr = *token;
                tmpstr.erase(
                    std::remove_if(
                        tmpstr.begin(), tmpstr.end(),
                        ::isspace),
                    tmpstr.end());
            }
            fields.insert(std::pair(nvctx, tmpstr));
        }
    }
    fclose(f);
}

std::vector<uint32_t> getAffinityList(int tid, int ncpus, int& nhwthr, std::string& tmpstr) {
    cpu_set_t mask;
    CPU_ZERO(&mask);
    auto msize = sizeof(mask);
    sched_getaffinity(tid, msize, &mask);
    nhwthr = CPU_COUNT(&mask);
    std::vector<uint32_t> allowed_list;
    for (int i = 0; i < ncpus ; i++) {
        // which hwthreads are in the set?
        if (CPU_ISSET(i, &mask)) {
            if (tmpstr.size() > 0) { tmpstr = tmpstr + ","; }
            tmpstr = tmpstr + std::to_string(i);
            allowed_list.push_back(i);
        }
    }
    return allowed_list;
}

std::string toString(std::set<uint32_t> allowed) {
    std::string outstr;
    for (auto t : allowed) {
        if (outstr.size() > 0) { outstr += ","; }
        outstr += std::to_string(t);
    }
    return outstr;
}

std::vector<std::map<std::string,std::string>> parseProcStat(void) {
    std::vector<std::map<std::string,std::string>> fields;
    FILE * pFile;
    char line[128];
    pFile = fopen ("/proc/stat","r");
    if (pFile == nullptr) {
        perror ("Error opening file");
        return fields;
    }
        while ( fgets( line, 128, pFile)) {
            // skip the total line
            if ( strncmp (line, "cpu ", 4) == 0 ) {
                continue;
            } else if ( strncmp (line, "cpu", 3) == 0 ) {
                std::vector<std::string> v;
                std::string tmp;
                std::stringstream ss(line);
                while (getline(ss, tmp, ' ')) {
                    // store token string in the vector
                    v.push_back(tmp);
                }
                std::map<std::string,std::string> f;
                f.insert(std::pair("user",v[1]));
                f.insert(std::pair("system",v[3]));
                f.insert(std::pair("idle",v[4]));
                fields.push_back(f);
            } else {
                // we're done at this point
                break;
            }
        }
    fclose (pFile);
    return fields;
}

void setThreadAffinity(int core) {
    cpu_set_t cpuset;
    cpu_set_t mask;
    if (sched_getaffinity(0, sizeof(cpu_set_t), &mask) == -1) {
        perror("sched_getaffinity");
        return;
    }
    if (CPU_ISSET(core, &mask)) {
        CPU_ZERO(&cpuset);
        CPU_SET(core, &cpuset);
        sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    }
    return;
}


}