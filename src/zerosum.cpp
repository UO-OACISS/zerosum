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

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <set>
#include <chrono>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fstream>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include "zerosum.h"
#include "perfstubs.h"
#include "utils.h"
#ifdef ZEROSUM_STANDALONE
#include "error_handling.h"
#ifdef ZEROSUM_USE_STATIC_GLOBAL_CONSTRUCTOR
#include "global_constructor_destructor.h"
extern "C" {
DEFINE_CONSTRUCTOR(zerosum_init_static_void)
DEFINE_DESTRUCTOR(zerosum_finalize_static_void)
}
#endif // ZEROSUM_USE_STATIC_GLOBAL_CONSTRUCTOR
#endif // ZEROSUM_STANDALONE

#ifdef ZEROSUM_USE_MPI
#include <mpi.h>
#define MPI_CALL(call) \
    do { \
        int _status = call; \
        if (_status != MPI_SUCCESS) { \
            char estring[MPI_MAX_ERROR_STRING]; \
            int resultlen; \
            MPI_Error_string(_status, estring, &resultlen); \
            fprintf(stderr, "%s:%d: error: function %s failed with error %s.\n", \
                    __FILE__, __LINE__, #call, estring); \
            MPI_Abort(MPI_COMM_WORLD, _status); \
        } \
    } while (0)
#else
#include <unistd.h>
#include <limits.h>
#endif
#ifdef USE_HWLOC
#include "hwloc_zs.h"
#endif

namespace zerosum {

std::mutex software::Process::thread_mtx;

using namespace std::literals::chrono_literals;

void ZeroSum::threadedFunction(void) {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    async_tid = gettid();
    setThreadAffinity(parseInt("ZS_ASYNC_CORE", process.getMaxHWT()));
    bool initialized = false;
    // We want to measure periodically, ON THE SECOND.
    // So, we take into consideration how long it takes to do
    // this measurement.
    auto prev = std::chrono::steady_clock::now();
    std::chrono::seconds period{parseInt("ZS_PERIOD", 1)};
    constexpr uint32_t oneYear = 60 * 60 * 24 * 365; // one year in seconds
    std::chrono::seconds timeLimit{parseInt("ZS_TIMELIMIT", oneYear)};
    auto expiration = prev + timeLimit;
    block_signal();
    while (working) {
        auto then = prev + period;
        auto stop = then - std::chrono::steady_clock::now();
        // keep trying until MPI is initialized
        while (!initialized && working) {
            initialized = doOnce();
        }
        // once initialized, we can do our periodic checks.
        if (initialized) {
            doPeriodic();
            logfile << process.logThreads() << std::flush;
        }
        std::unique_lock<std::mutex> lk(cv_m);
        if(cv.wait_for(lk, stop, [&]{return !working;}))
            return;
        prev = then;
        // check for expiration date
        if (std::chrono::steady_clock::now() > expiration) {
            finalizeLog();
            pthread_kill(this->process.id, SIGQUIT);
        }
    }
}

#include "banner.h"

inline void ZeroSum::getMPIinfo(void) {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    int size, rank;
#ifdef ZEROSUM_USE_MPI
    // get mpi info
    MPI_CALL(MPI_Comm_size(MPI_COMM_WORLD, &size));
    MPI_CALL(MPI_Comm_rank(MPI_COMM_WORLD, &rank));
    char name[MPI_MAX_PROCESSOR_NAME];
    int resultlength;
    MPI_CALL(MPI_Get_processor_name(name, &resultlength));
#else
    size = 1;
    rank = 0;
    char name[HOST_NAME_MAX];
    gethostname(name, HOST_NAME_MAX);
#endif
    computeNode = hardware::ComputeNode(name, doDetails);
    process.rank = rank;
    process.size = size;
#ifdef ZEROSUM_USE_STATIC_GLOBAL_CONSTRUCTOR
    /* We can't call std::cout from a static global constructor, which is
     * where the thread that this function is spawned from starts
     * its life when run without MPI. */
    if (rank == 0) { printf("%s\n", ghost_banner); }
#else
    if (rank == 0) { std::cout << ghost_banner << std::endl; }
#endif
}

inline void ZeroSum::openLog(void) {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    // open a log file
    std::string filename{"zs."};
    // prefix the rank with as many zeros as needed to sort correctly.
    static bool use_pid{parseBool("ZS_USE_PID",false)};
    size_t len{1};
    int precision{5};
    std::string tmp;
    if (use_pid) {
        len = std::to_string(parseMaxPid()).size();
        printf("Got len: %lu\n", len);
        tmp = std::to_string(process.id);
        precision = len - std::min(len,tmp.size());
        filename += computeNode.name;
        filename += ".";
    } else {
        len = std::to_string(process.size-1).size();
        tmp = std::to_string(process.rank);
        precision = len - std::min(len,tmp.size());
    }
    tmp.insert(0, precision, '0');
    filename += tmp;
    filename += ".log";
    //std::cout << "Opening log file: " << filename << std::endl;
    logfile.open(filename);
}

bool ZeroSum::doOnce(void) {
    static bool done{false};
    if (done) return done;
    PERFSTUBS_SCOPED_TIMER_FUNC();

#ifdef ZEROSUM_USE_MPI
    int ready;
    MPI_CALL(MPI_Initialized(&ready));
    if (!ready) return done;
#endif

    getMPIinfo();
    openLog();
    logfile << process.toString() << std::flush;
    getgpu();
    computeNode.updateFields(parseNodeInfo());
#ifdef USE_HWLOC
    ScopedHWLOC::validate_hwloc(process.rank);
#endif
    done = true;
    return done;
}

/* If the user creates a file named "zerosum.stop", dump everything and exit */
void ZeroSum::checkForStop(void) {
    if(access("./zerosum.stop", F_OK) == 0) {
        std::cerr << "Stop file detected! Aborting!" << std::endl;
        std::cerr << "Thread " << gettid() << " signalling " << this->process.id << std::endl;
        finalizeLog();
        pthread_kill(this->process.id, SIGQUIT);
    }
    static bool monitorLog{parseBool("ZS_MONITOR_LOG", false)};
    if (monitorLog) {
        static std::string logfileName = parseString("ZS_MONITOR_LOG_FILENAME", "log.out");
        static int duration = parseInt("ZS_MONITOR_LOG_TIMEOUT", 300);
        struct stat fileStat;
        if (stat(logfileName.c_str(), &fileStat) == 0) {
            // Get the current time
            time_t currentTime;
            time(&currentTime);

            // Calculate the time difference in seconds
            long timeDifference = difftime(currentTime, fileStat.st_mtime);
            if (process.rank == 0) {
                if (logfile.is_open()) {
                    logfile << "logfile time freshness: " << timeDifference << " seconds" << std::endl;
                }
            }

            // Check if the file has been updated in the last N seconds
            if(timeDifference > duration) {
                std::cerr << "Log file " << logfileName << " went stale! Aborting!" << std::endl;
                std::cerr << "Thread " << gettid() << " signalling " << this->process.id << std::endl;
                finalizeLog();
                pthread_kill(this->process.id, SIGQUIT);
            }
        }
    }
}

void ZeroSum::doPeriodic(void) {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    step++;
    getpthreads();
    computeNode.updateFields(parseProcStat(),step);
    computeNode.updateFields(parseNodeInfo());
    getgpustatus();
    std::string tmpstr{computeNode.reportMemory()};
    logfile << tmpstr << std::flush;
    if (process.rank == 0 && getHeartBeat()) {
        std::cout << tmpstr << std::flush;
    }
    checkForStop();
}

void ZeroSum::getProcStatus() {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    std::string allowed_string = getCpusAllowed("/proc/self/status");
    std::vector<uint32_t> allowed_list = parseDiscreteValues(allowed_string);
    std::string filename = "/proc/self/stat";
    auto fields = getThreadStat(filename.c_str());
    filename = "/proc/self/status";
    getThreadStatus(filename.c_str(), fields);
    //fields.insert(std::pair("step",std::to_string(step)));
    process = software::Process(getpid(), 0, 1, fields, allowed_list);
    process.hwthreads_raw = allowed_string;
    process.computeNode = &computeNode;
    if (doDetails) {
        getOtherProcesses();
    }
    return;
}

/* The main singleton constructor for the ZeroSum class */
ZeroSum::ZeroSum(void) : step(0), start(std::chrono::steady_clock::now()), doShutdown(true) {
    working = true;
    if (parseBool("ZS_SIGNAL_HANDLER", false)) {
        register_signal_handler();
    }
    PERFSTUBS_INITIALIZE();
    doDetails = parseBool("ZS_DETAILS", false);

    /* Important to do this now, before OpenMP is initialized
     * and this thread gets pinned to any cores */
    getProcStatus();
    computeNode.updateFields(parseProcStat(),step);
    /* Make sure we query the node with Hwloc before we launch the thread */
    worker = std::thread{&ZeroSum::threadedFunction, this};
    // increase the step, because the main thread will be one of the OpenMP threads.
    step++;
#ifdef ZEROSUM_USE_OPENMP
    if (parseBool("ZS_USE_OPENMP", false)) {
        getopenmp();
    }
#endif
    //worker.detach();
}

void ZeroSum::handleCrash(void) {
    //auto end = std::chrono::steady_clock::now();
    //std::chrono::duration<double> diff = end - start;
    //std::cerr << "\nDuration of execution: " << diff.count() << " s\n";
    //std::cerr << process.getSummary() << std::endl;
    doShutdown = false;
}

void ZeroSum::shutdown(void) {
    if (!doShutdown) return;
    working = false;
    cv.notify_all();
    worker.join();
    if (process.rank == 0) {
        // record end time
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = end - start;
        std::cout << "\nDuration of execution: " << diff.count() << " s\n";
        std::cout << process.getSummary() << std::endl;
        if (otherProcesses.size() > 0) {
            std::cout << "Other processes:\n";
            for (auto p : otherProcesses) {
                std::cout << p.getSummary(false);
            }
            std::cout << std::endl;
        }
    }
    finalizeLog();
    PERFSTUBS_FINALIZE();
}

void ZeroSum::finalizeLog() {
    if (logfile.is_open()) {
        logfile << process.logThreads(true) << std::flush;
        logfile << computeNode.toString(process.hwthreads) << std::flush;
        logfile << process.toString() << std::flush;
        logfile.close();
    }
}

std::pair<std::string,std::string> split (const std::string &s) {
    char delim{'='};
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;
    while (getline (ss, item, delim)) {
        result.push_back (item);
    }
    return std::pair(result[0],result[1]);
}

void ZeroSum::parseEnv(char** envp) {
    for (int i = 0; envp[i] != NULL; i++) {
        if (strncmp(envp[i], "OMP_", 4) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "HIP_", 4) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "ROC", 3) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strstr(envp[i], "VISIBLE_DEVICES") != NULL) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "CUDA_", 5) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "OMPI_", 5) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "MPICH_", 6) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "MPIR_", 5) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "SLURM_", 6) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "CRAY_", 5) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "ZE_", 3) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "PBS_", 4) == 0) {
            process.environment.insert(split(envp[i]));
        }
    }
}

void ZeroSum::recordSentBytes(int rank, size_t bytes) {
    process.recordSentBytes(rank, bytes);
}

void ZeroSum::recordRecvBytes(int rank, size_t bytes) {
    process.recordRecvBytes(rank, bytes);
}

} // namespace zerosum

#ifdef ZEROSUM_STANDALONE
#ifdef ZEROSUM_USE_STATIC_GLOBAL_CONSTRUCTOR
void zerosum_init_static_void(void){
    zerosum::ZeroSum::getInstance();
}

void zerosum_finalize_static_void(void){
    zerosum::ZeroSum::getInstance().shutdown();
}
#else
#include "preload.cpp"
#endif
#endif


