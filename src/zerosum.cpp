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
        }
        sleep(1);
    }
}

bool ZeroSum::doOnce(void) {
    static bool done{false};
    if (done) return done;

    int ready;
    MPI_CALL(MPI_Initialized(&ready));
    if (ready) {
        int resultlength;
        MPI_Comm_size(MPI_COMM_WORLD, &size);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Get_processor_name(name, &resultlength);
        // get mpi info
        getgpu(rank, section++, name);
        getProcStatus(section++);
        ncpus = std::thread::hardware_concurrency();
        getopenmp(rank, section++, ncpus, tids);
        done = true;
    }
    return done;
}

void ZeroSum::doPeriodic(void) {
    getpthreads(rank, section++, ncpus, tids);
}


ZeroSum::ZeroSum(void) {
    section = 0;
    ncpus = 1;
    working = true;
    worker = std::thread{&ZeroSum::threadedFunction, this};
    //worker.detach();
}

void ZeroSum::shutdown(void) {
    working = false;
    worker.join();
}

void zerosum_init_static_void(void){
    ZeroSum::getInstance();
}

void zerosum_finalize_static_void(void){
    ZeroSum::getInstance().shutdown();
}


