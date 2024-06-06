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
#include <set>
#include <thread>
#include <iostream>
#include <fstream>
#include <atomic>
#include <condition_variable>
#include "topology.h"
#ifdef USE_ADIOS2
#include <adios2.h>
#endif //USE_ADIOS2

namespace zerosum {

class ZeroSum {
public:
    static ZeroSum& getInstance() {
        static ZeroSum instance;
        return instance;
    }

    // Other public member functions...
    void finalizeLog(void);
    void shutdown(void);
    void handleCrash(void);
    void parseEnv(char** envp);
    uint32_t getRank(void) { return process.rank; }
    void recordSentBytes(int rank, size_t bytes);
    void recordRecvBytes(int rank, size_t bytes);
    software::Process& getProcess(void) { return process; }
    hardware::ComputeNode getComputeNode(void) { return computeNode; }
    std::ofstream& getLogfile(void) { return logfile; }
#ifdef USE_ADIOS2
    adios2::fstream& getAdiosFile(void) { return adiosFile; }
#endif //USE_ADIOS2

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
#ifdef USE_ADIOS2
    adios2::fstream adiosFile;
#endif //USE_ADIOS2
    software::Process process;
    std::vector<software::Process> otherProcesses;
    hardware::ComputeNode computeNode;
    uint32_t async_tid;
    std::atomic<uint32_t> step;
    std::condition_variable cv;
    std::mutex cv_m;
    std::chrono::time_point<std::chrono::steady_clock> start;
    bool doShutdown;
    bool doDetails;

    // Other private member variables and functions...
    void getMPIinfo(void);
    void openLog(void);
    int getgpu(void);
    int getgpustatus(void);
#ifdef ZEROSUM_USE_OPENMP
    void getopenmp(void);
#endif
    int getpthreads(void);
    void getProcStatus(void);
    void threadedFunction(void);
    bool doOnce(void);
    void doPeriodic(void);
    void checkForStop(void);
    int getOtherProcesses(void);
};

} // namespace zerosum
