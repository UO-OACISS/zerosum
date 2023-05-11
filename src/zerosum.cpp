/**********************************************************
Inspired by Tom Papatheodore
Written by Kevin Huck
**********************************************************/

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
#include "zerosum.h"
#include "perfstubs.h"
#include "utils.h"
#ifdef ZEROSUM_STANDALONE
#ifdef ZEROSUM_USE_STATIC_GLOBAL_CONSTRUCTOR
#include "global_constructor_destructor.h"
extern "C" {
DEFINE_CONSTRUCTOR(zerosum_init_static_void)
DEFINE_DESTRUCTOR(zerosum_finalize_static_void)
}
#endif // ZEROSUM_USE_STATIC_GLOBAL_CONSTRUCTOR
#endif // ZEROSUM_STANDALONE

#ifdef USE_MPI
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

namespace zerosum {

using namespace std::literals::chrono_literals;

void ZeroSum::threadedFunction(void) {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    async_tid = gettid();
    setThreadAffinity(process.getMaxHWT());
    bool initialized = false;
    // We want to measure periodically, ON THE SECOND.
    // So, we take into consideration how long it takes to do
    // this measurement.
    auto prev = std::chrono::steady_clock::now();
    while (working) {
        auto then = prev + 1s;
        auto stop = then - std::chrono::steady_clock::now();
        std::unique_lock<std::mutex> lk(cv_m);
        if(cv.wait_for(lk, stop, [&]{return !working;}))
            return;
        // keep trying until MPI is initialized
        if (!initialized) {
            initialized = doOnce();
        }
        // once initialized, we can do our periodic checks.
        if (initialized) {
            doPeriodic();
            logfile << process.logThreads() << std::flush;
        }
        prev = then;
    }
}

inline void ZeroSum::getMPIinfo(void) {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    int size, rank;
#ifdef USE_MPI
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
    computeNode = hardware::ComputeNode(name);
    process.rank = rank;
    process.size = size;
}

inline void ZeroSum::openLog(void) {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    // open a log file
    std::string filename{"zs."};
    filename += std::to_string(process.rank);
    filename += ".log";
    //std::cout << "Opening log file: " << filename << std::endl;
    logfile.open(filename);
}

bool ZeroSum::doOnce(void) {
    static bool done{false};
    if (done) return done;
    PERFSTUBS_SCOPED_TIMER_FUNC();

#ifdef USE_MPI
    int ready;
    MPI_CALL(MPI_Initialized(&ready));
    if (!ready) return done;
#endif

    getMPIinfo();
    openLog();
    logfile << process.toString() << std::flush;
    getgpu();
    //getopenmp();
    done = true;
    return done;
}

void ZeroSum::doPeriodic(void) {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    step++;
    getpthreads();
    computeNode.updateFields(parseProcStat(),step);
}

void ZeroSum::getProcStatus() {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    std::string allowed_string = getCpusAllowed("/proc/self/status");
    //std::cout << "/proc/self/status : " << allowed_string << std::endl;
    std::vector<uint32_t> allowed_list = parseDiscreteValues(allowed_string);
    std::string filename = "/proc/self/stat";
    auto fields = getThreadStat(filename.c_str());
    filename = "/proc/self/status";
    getThreadStatus(filename.c_str(), fields);
    fields.insert(std::pair("step",std::to_string(step)));
    process = software::Process(getpid(), 0, 1, fields, allowed_list);
    process.hwthreads_raw = allowed_string;
    process.computeNode = &computeNode;
    return;
}

/* The main singleton constructor for the ZeroSum class */
ZeroSum::ZeroSum(void) : step(0) {
    working = true;
    PERFSTUBS_INITIALIZE();
    /* Important to do this now, before OpenMP is initialized
     * and this thread gets pinned to any cores */
    getProcStatus();
    computeNode.updateFields(parseProcStat(),step);
    worker = std::thread{&ZeroSum::threadedFunction, this};
    // increase the step, because the main thread will be one of the OpenMP threads.
    step++;
    getopenmp();
    //worker.detach();
}

void ZeroSum::shutdown(void) {
    working = false;
    cv.notify_all();
    worker.join();
    logfile << process.logThreads(true) << std::flush;
    logfile << computeNode.toString(process.hwthreads) << std::flush;
    if (logfile.is_open()) {
        logfile.close();
    }
    if (process.rank == 0) std::cout << process.getSummary() << std::endl;
    PERFSTUBS_FINALIZE();
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
        else if (strncmp(envp[i], "OMPI_", 5) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "SLURM_", 6) == 0) {
            process.environment.insert(split(envp[i]));
        }
        else if (strncmp(envp[i], "CRAY_", 5) == 0) {
            process.environment.insert(split(envp[i]));
        }
    }
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


