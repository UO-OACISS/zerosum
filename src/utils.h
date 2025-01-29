/*
 * MIT License
 *
 * Copyright (c) 2023 University of Oregon, Kevin Huck
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include <string>
#include <set>
#include <map>
#include <vector>
#include <cstdint>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#define UNUSED(expr) do { (void)(expr); } while (0)

#define handle_error_en(en, msg) \
               do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

namespace zerosum {

std::vector<uint32_t> parseDiscreteValues(std::string inputString);
std::string getCpusAllowed(const char * filename);
std::map<std::string, std::string> getThreadStat(const char * filename);
void getThreadStatus(const char * filename, std::map<std::string, std::string>& fields);
bool isRunning(std::map<std::string, std::string>& fields, const char * tid, bool isMain);
std::vector<uint32_t> getAffinityList(int tid, int ncpus, int& nhwthr, std::string& tmpstr);
std::string toString(std::set<uint32_t> allowed);
std::vector<std::map<std::string,std::string>> parseProcStat(void);
std::map<std::string,std::string> parseNodeInfo(void);
void setThreadAffinity(int core);
bool parseBool(const char * env, bool default_value);
int parseInt(const char * env, int default_value);
std::string parseString(const char * env, std::string default_value);
bool getVerbose(void);
bool getHeartBeat(void);
size_t parseMaxPid(void);
std::string getUniqueFilename(void);
int test_for_MPI_comm_rank(int commrank);
int test_for_MPI_comm_size(int commsize);
int test_for_MPI_local_rank(int commrank);

class in_zs {
    public:
        static size_t& get() {
            thread_local static size_t _in{0};
            return _in;
        }
        in_zs(){ get()++; };
        ~in_zs(){ get()--; };
};

}
