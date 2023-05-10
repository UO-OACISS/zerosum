#pragma once
#include <string>
#include <set>
#include <map>
#include <vector>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

namespace zerosum {

std::vector<uint32_t> parseDiscreteValues(std::string inputString);
std::string getCpusAllowed(const char * filename);
std::map<std::string, std::string> getThreadStat(const char * filename);
void getThreadStatus(const char * filename, std::map<std::string, std::string>& fields);
std::vector<uint32_t> getAffinityList(int tid, int ncpus, int& nhwthr, std::string& tmpstr);
std::string toString(std::set<uint32_t> allowed);
std::vector<std::map<std::string,std::string>> parseProcStat(void);

}