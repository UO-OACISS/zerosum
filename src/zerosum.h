#pragma once
#include <set>
#include <thread>
#include <iostream>
#include <fstream>
#include <atomic>
#include <condition_variable>
#include "topology.h"

namespace zerosum {

class ZeroSum {
public:
    static ZeroSum& getInstance() {
        static ZeroSum instance;
        return instance;
    }

    // Other public member functions...
    void shutdown(void);
    void handleCrash(void);
    void parseEnv(char** envp);
    uint32_t getRank(void) { return process.rank; }

private:
    /* Standard singleton definition follows */
    ZeroSum();
    ~ZeroSum() {
        if (working) {
            shutdown();
        }
        if (worker.joinable()) {
            worker.detach();
        }
    };
    ZeroSum(const ZeroSum&) = delete;
    ZeroSum& operator=(const ZeroSum&) = delete;
    ZeroSum(ZeroSum&&) = delete;
    ZeroSum& operator=(ZeroSum&&) = delete;

    /* member variables */

    std::thread worker; // the asynchronous thread
    bool working;
    std::ofstream logfile;
    software::Process process;
    hardware::ComputeNode computeNode;
    uint32_t async_tid;
    std::atomic<uint32_t> step;
    std::condition_variable cv;
    std::mutex cv_m;
    std::chrono::time_point<std::chrono::steady_clock> start;
    bool doShutdown;

    // Other private member variables and functions...
    void getMPIinfo(void);
    void openLog(void);
    int getgpu(void);
    int getgpustatus(void);
    void getopenmp(void);
    int getpthreads(void);
    void getProcStatus(void);
    void threadedFunction(void);
    bool doOnce(void);
    void doPeriodic(void);
};

} // namespace zerosum
