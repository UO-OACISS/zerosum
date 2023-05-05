/**********************************************************
Inspired by Tom Papatheodore
Written by Kevin Huck
**********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <set>
#include <mpi.h>
#include <unistd.h>
#include "zerosum.h"
#include "perfstubs.h"
#ifdef ZEROSUM_STANDALONE
#include "global_constructor_destructor.h"
extern "C" {
DEFINE_CONSTRUCTOR(zerosum_init_static_void)
DEFINE_DESTRUCTOR(zerosum_finalize_static_void)
}
#endif // ZEROSUM_STANDALONE

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

void ZeroSum::threadedFunction(void) {
    while (working) {
        if (doOnce()) {
            doPeriodic();
            step++;
        }
        sleep(1);
    }
}

bool ZeroSum::doOnce(void) {
    static bool done{false};
    if (done) return done;
    PERFSTUBS_SCOPED_TIMER_FUNC();

    int ready;
    MPI_CALL(MPI_Initialized(&ready));
    if (ready) {
        int resultlength;
        // get mpi info
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Get_processor_name(name, &resultlength);
        // open a log file
        std::string filename{"zs."};
        filename += std::to_string(rank);
        filename += ".log";
        logfile.open(filename);
        getgpu(rank, 0, name);
        getProcStatus(1);
        logfile << earlyData;
        done = true;
    }
    return done;
}

void ZeroSum::doPeriodic(void) {
    PERFSTUBS_SCOPED_TIMER_FUNC();
    getpthreads(rank, 3, ncpus, tids);
}


ZeroSum::ZeroSum(void) {
    step = 0;
    section = 0;
    ncpus = 1;
    working = true;
    PERFSTUBS_INITIALIZE();
    ncpus = std::thread::hardware_concurrency();
    earlyData = getopenmp(rank, 2, ncpus, tids);
    worker = std::thread{&ZeroSum::threadedFunction, this};
    //worker.detach();
}

void ZeroSum::shutdown(void) {
    working = false;
    worker.join();
    if (logfile.is_open()) {
        logfile.close();
    }
    PERFSTUBS_FINALIZE();
}

void zerosum_init_static_void(void){
    ZeroSum::getInstance();
}

void zerosum_finalize_static_void(void){
    ZeroSum::getInstance().shutdown();
}


