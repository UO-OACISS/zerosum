/*
 * MIT License
 *
 * Copyright (c) 2023-2025 University of Oregon, Kevin Huck
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
#include <unordered_map>
#include <string.h>
#include "zerosum.h"

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
    // fields.insert(std::pair("executable", v[2])); // always 0
    fields.insert(std::pair("state", v[3]));
    fields.insert(std::pair("minflt", v[10]));
    fields.insert(std::pair("majflt", v[12]));
    fields.insert(std::pair("utime", v[14]));
    fields.insert(std::pair("stime", v[15]));
    // fields.insert(std::pair("startstack", v[28])); // probably not available without ptrace, always 0
    // fields.insert(std::pair("kstkesp", v[29])); // probably not available without ptrace, always 0
    // fields.insert(std::pair("kstkeip", v[30])); // probably not available without ptrace, always 0
    fields.insert(std::pair("nswap", v[36]));
    fields.insert(std::pair("processor", v[39]));
    fclose(f);
    return fields;
}

// Return true if the thread is running, false otherwise.
/* The state will be one of:
    (3) state  %c
         One of the following characters, indicating process
         state:
         R      Running
         S      Sleeping in an interruptible wait
         D      Waiting in uninterruptible disk sleep
         Z      Zombie
         T      Stopped (on a signal) or (before Linux
                2.6.33) trace stopped
         t      Tracing stop (Linux 2.6.33 onward)
         W      Paging (only before Linux 2.6.0)
         X      Dead (from Linux 2.6.0 onward)
         x      Dead (Linux 2.6.33 to 3.13 only)
         K      Wakekill (Linux 2.6.33 to 3.13 only)
         W      Waking (Linux 2.6.33 to 3.13 only)
         P      Parked (Linux 3.9 to 3.13 only)
         I      Idle (Linux 4.14 onward)
*/
bool isRunning(std::map<std::string, std::string>& fields,
    const char * tid, bool isMain) {
    /* We need a static variable to keep track of the last minor fault value,
       because it's a monotonically increasing value on some systems.
       so we want to see if it has increased in the last time quantum. */
    static std::unordered_map<std::string, uint64_t> priorMinflt;
    static bool deadlock{parseBool("ZS_DETECT_DEADLOCK",false)};
    if (!deadlock) {return true;}
    const std::string state{"state"};
    const std::string minflt{"minflt"};
    const std::string utime{"utime"};
    const std::string running{"R"};
    const std::string tracingStop{"t"};
    const std::string zeroFaults{"0"};
    std::string tmptid(tid);
    /* If the thread state is Running (R), and the minflt value is non-zero, then we are running.
     * Why the minflt? Because MPI will busy wait, which looks like running. But running with
     * no minor faults is highly unlikely.
     * We also check for the tracing stop state (t), because that seems to be related to
     * GPU processing on AMD machines.
     */
    if (fields.count(state) > 0) {
        // ok, the thread claims to be running (state == R)
        if (running.compare(fields[state]) == 0) {
            // if not the main thread, and it says it's running, it's running.
            if (!isMain) { return true; }
            uint64_t newMinflt = atol(fields[minflt].c_str());
            // have we seen this thread's minflt state before? if not, save it.
            if (priorMinflt.find(tmptid) == priorMinflt.end()) {
                priorMinflt[tmptid] = newMinflt;
                return true;
            }
            // a "stuck" thread:
            // minflt will be 0 for non-monotonically increasing machines,
            // it will be equal to the previous for monotonically increasing
            if (newMinflt == 0 || priorMinflt[tmptid] == newMinflt) {
                priorMinflt[tmptid] = newMinflt;
                return false;
            }
            // assume it's running for reals
            priorMinflt[tmptid] = newMinflt;
            return true;
        }
        // this is also a "running" state - it's tracing
        if (tracingStop.compare(fields[state]) == 0) { return true; }
        // if it occupied the CPU some time in the last period, it's running.
        if (!isMain && atol(fields[utime].c_str()) > 0) { return true; }
    }
    // neither running nor tracing? not running.
    return false;
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
        while ( fgets( line, 127, pFile)) {
            // skip the total line
            if ( strncmp (line, "cpu ", 4) == 0 ) {
                continue;
            } else if ( strncmp (line, "cpu", 3) == 0 ) {
                std::vector<unsigned int> v;
                std::string tmp;
                std::stringstream ss(line);
                while (getline(ss, tmp, ' ')) {
                    // store token string in the vector
                    v.push_back(atol(tmp.c_str()));
                }
                // These "corrections" are what htop does...
                // see https://github.com/htop-dev/htop/blob/4102862d12695cdf003e2d51ef6ce5984b7136d7/linux/LinuxMachine.c#L455
                v[1] -= v[9]; // user -= guest
                v[2] -= v[10]; // nice -= guest_nice
                double idle_all_time = v[4] + v[5]; //idletime + ioWait;
                double system_all_time = v[3] + v[6] + v[7]; //systemtime + irq + softIrq;
                double virt_all_time = v[9] + v[10]; //guest + guestnice;
                double total_time = v[1] + v[2] + system_all_time + idle_all_time + v[8] + virt_all_time; //usertime + nicetime + system_all_time + idle_all_time + steal + virt_all_time;
                std::map<std::string,std::string> f;
                f.insert(std::pair("user",std::to_string(v[1])));
                f.insert(std::pair("nice",std::to_string(v[2])));
                f.insert(std::pair("system",std::to_string(v[3])));
                f.insert(std::pair("system_all",std::to_string(system_all_time)));
                f.insert(std::pair("idle",std::to_string(v[4])));
                f.insert(std::pair("idle_all",std::to_string(idle_all_time)));
                f.insert(std::pair("iowait",std::to_string(v[5])));
                f.insert(std::pair("irq",std::to_string(v[6])));
                f.insert(std::pair("softirq",std::to_string(v[7])));
                f.insert(std::pair("steal",std::to_string(v[8])));
                f.insert(std::pair("guest",std::to_string(v[9])));
                f.insert(std::pair("virt_all_time",std::to_string(virt_all_time)));
                f.insert(std::pair("guest_nice",std::to_string(v[10])));
                f.insert(std::pair("total_time",std::to_string(total_time)));
                fields.push_back(f);
            } else {
                // we're done at this point
                break;
            }
        }
    fclose (pFile);
    return fields;
}

std::map<std::string,std::string> parseNodeInfo(void) {
    std::map<std::string,std::string> fields;
    FILE * pFile;
    char line[128];
    pFile = fopen ("/proc/meminfo","r");
    if (pFile == nullptr) {
        perror ("Error opening file");
        return fields;
    }
    while ( fgets( line, 128, pFile)) {
        std::string tmp(line);
        const std::regex separator(":");
        std::sregex_token_iterator token(tmp.begin(), tmp.end(),
            separator, -1);
        std::sregex_token_iterator end;
        std::string name = *token++;
        if (token != end && name.rfind("Mem", 0) == 0) {
            name += " kB";
            std::string value = *token;
            unsigned d1 = strtoul (value.c_str(), nullptr, 0);
            fields.insert(std::pair<std::string,std::string>(name,std::to_string(d1)));
        }
    }
    fclose(pFile);
    return fields;
}

size_t parseMaxPid(void) {
    size_t maxpid{0};
    FILE * pFile;
    char line[128];
    pFile = fopen ("/proc/sys/kernel/pid_max","r");
    if (pFile == nullptr) {
        perror ("Error opening file");
        return maxpid;
    }
    while ( fgets( line, 128, pFile)) {
        maxpid = atol(line);
    }
    fclose(pFile);
    return maxpid;
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
        if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
            perror("sched_setaffinity");
        }
    }
    return;
}

/*********************************************************************
 * Parse an integer value
 ********************************************************************/
int parseInt(const char *env, int default_value = 0) {
    const char * str = getenv(env);
    if (str == NULL) { return default_value; }
    int tmp = atoi(str);
    if (tmp < 0) { return default_value; }
    return tmp;
}

/*********************************************************************
 * Parse a string value
 ********************************************************************/
std::string parseString(const char * env, std::string default_value) {
    const char * str = getenv(env);
    if (str == NULL) { return default_value; }
    std::string tmp{str};
    return tmp;
}

/*********************************************************************
 * Parse a boolean value
 ********************************************************************/
bool parseBool(const char * env, bool default_value = false) {
    const char * str = getenv(env);
    if (str == NULL) {
        return default_value;
    }
    static char strbuf[128];
    char *ptr = strbuf;
    strncpy(strbuf, str, 127);
    while (*ptr) {
        *ptr = tolower(*ptr);
        ptr++;
    }
    if (strcmp(strbuf, "yes") == 0  ||
        strcmp(strbuf, "true") == 0 ||
        strcmp(strbuf, "on") == 0 ||
        strcmp(strbuf, "1") == 0) {
        return 1;
    } else {
        return 0;
    }
}

bool getVerbose(void) {
    static bool verbose{parseBool("ZS_VERBOSE",false)};
    return verbose;
}

bool getHeartBeat(void) {
    static bool verbose{parseBool("ZS_HEART_BEAT",false)};
    return verbose;
}

std::string getUniqueFilename(void) {
    std::string filename;
    std::string tmp;
    static bool use_pid{parseBool("ZS_USE_PID",false)};
    size_t len{1};
    int precision{5};
    if (use_pid) {
        len = std::to_string(parseMaxPid()).size();
        printf("Got len: %lu\n", len);
        tmp = std::to_string(ZeroSum::getInstance().getPid());
        precision = len - std::min(len,tmp.size());
        filename += ZeroSum::getInstance().getHostname();
        filename += ".";
    } else {
        len = std::to_string(ZeroSum::getInstance().getSize()-1).size();
        tmp = std::to_string(ZeroSum::getInstance().getRank());
        precision = len - std::min(len,tmp.size());
    }
    tmp.insert(0, precision, '0');
    filename += tmp;
    return filename;
}

int test_for_MPI_local_rank(int commrank) {
    /* Some configurations might use MPI without telling ZeroSum - they can
     * call apex::init() with a rank of 0 and size of 1 even though
     * they are running in an MPI application.  For that reason, we double
     * check to make sure that we aren't in an MPI execution by checking
     * for some common environment variables. */
    // PMI, MPICH, Cray, Intel, MVAPICH2...
    const char * tmpvar = getenv("PMI_LOCAL_RANK");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        // printf("Changing PMI rank to %lu\n", commrank);
        return commrank;
    }
    // PALS
    tmpvar = getenv("PALS_LOCAL_RANKID");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        // printf("Changing PALS rank to %lu\n", commrank);
        return commrank;
    }
    // OpenMPI, Spectrum
    tmpvar = getenv("OMPI_COMM_WORLD_LOCAL_RANK");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        // printf("Changing openMPI rank to %lu\n", commrank);
        return commrank;
    }
    // Slurm - last resort
    tmpvar = getenv("SLURM_LOCALID");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        return commrank;
    }
    return commrank;
}

int test_for_MPI_comm_rank(int commrank) {
    /* Some configurations might use MPI without telling ZeroSum - they can
     * call apex::init() with a rank of 0 and size of 1 even though
     * they are running in an MPI application.  For that reason, we double
     * check to make sure that we aren't in an MPI execution by checking
     * for some common environment variables. */
    // PMI, MPICH, Cray, Intel, MVAPICH2...
    const char * tmpvar = getenv("PMI_RANK");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        // printf("Changing PMI rank to %lu\n", commrank);
        return commrank;
    }
    // cray ALPS
    tmpvar = getenv("ALPS_APP_PE");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        // printf("Changing ALPS rank to %lu\n", commrank);
        return commrank;
    }
    // PALS
    tmpvar = getenv("PALS_RANKID");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        // printf("Changing PALS rank to %lu\n", commrank);
        return commrank;
    }
    // cray PMI
    tmpvar = getenv("CRAY_PMI_RANK");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        // printf("Changing CRAY_PMI rank to %lu\n", commrank);
        return commrank;
    }
    // OpenMPI, Spectrum
    tmpvar = getenv("OMPI_COMM_WORLD_RANK");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        // printf("Changing openMPI rank to %lu\n", commrank);
        return commrank;
    }
    // PBS/Torque
    tmpvar = getenv("PBS_TASKNUM");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        return commrank;
    }
    // PBS/Torque on some systems...
    tmpvar = getenv("PBS_O_TASKNUM");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar) - 1;
        return commrank;
    }
    // Slurm - last resort
    tmpvar = getenv("SLURM_PROCID");
    if (tmpvar != NULL) {
        commrank = atol(tmpvar);
        return commrank;
    }
    return commrank;
}

int test_for_MPI_comm_size(int commsize) {
    /* Some configurations might use MPI without telling ZeroSum - they can
     * call apex::init() with a rank of 0 and size of 1 even though
     * they are running in an MPI application.  For that reason, we double
     * check to make sure that we aren't in an MPI execution by checking
     * for some common environment variables. */
    // PMI, MPICH, Cray, Intel, MVAPICH2...
    const char * tmpvar = getenv("PMI_SIZE");
    if (tmpvar != NULL) {
        commsize = atol(tmpvar);
        // printf("Changing MPICH size to %lu\n", commsize);
        return commsize;
    }
    tmpvar = getenv("CRAY_PMI_SIZE");
    if (tmpvar != NULL) {
        commsize = atol(tmpvar);
        // printf("Changing MPICH size to %lu\n", commsize);
        return commsize;
    }
    tmpvar = getenv("PALS_SIZE");
    if (tmpvar != NULL) {
        commsize = atol(tmpvar);
        // printf("Changing PALS size to %lu\n", commsize);
        return commsize;
    }
    // OpenMPI, Spectrum
    tmpvar = getenv("OMPI_COMM_WORLD_SIZE");
    if (tmpvar != NULL) {
        commsize = atol(tmpvar);
        // printf("Changing openMPI size to %lu\n", commsize);
        return commsize;
    }
    // PBS/Torque - no specific variable specifies number of nodes?
    tmpvar = getenv("PBS_NP"); // number of processes requested
    if (tmpvar != NULL) {
        commsize = atol(tmpvar);
        return commsize;
    }
    // PBS/Torque - no specific variable specifies number of nodes?
    tmpvar = getenv("PBS_NUM_NODES"); // number of nodes requested
    const char * tmpvar2 = getenv("PBS_NUM_PPN"); // number of processes per node
    if (tmpvar != NULL && tmpvar2 != NULL) {
        commsize = atol(tmpvar) * atol(tmpvar2);
        return commsize;
    }
    // Slurm - last resort
    tmpvar = getenv("SLURM_NPROCS");
    if (tmpvar != NULL) {
        commsize = atol(tmpvar);
        return commsize;
    }
    // Slurm - last resort
    tmpvar = getenv("SLURM_NTASKS");
    if (tmpvar != NULL) {
        commsize = atol(tmpvar);
        return commsize;
    }
    // Slurm - last resort, do some math
    tmpvar = getenv("SLURM_JOB_NUM_NODES");
    tmpvar2 = getenv("SLURM_TASKS_PER_NODE");
    if (tmpvar != NULL && tmpvar2 != NULL) {
        commsize = atol(tmpvar) * atol(tmpvar2);
        return commsize;
    }
    return commsize;
}

}
