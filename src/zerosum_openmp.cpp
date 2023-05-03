#include "zerosum.h"
#ifndef _GNU_SOURCE
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif
#define _XOPEN_SOURCE 700
#include <sched.h>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <set>
#include <iomanip>
#include <iomanip>
#include <string.h>
#include <mpi.h>
#include <omp.h>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#include <set>
#include "omp.h"

int ZeroSum::getopenmp(const int rank, const int section, const int ncpus, std::set<long>& tids) {
    int hwthread;
    int thread_id = 0;

#pragma omp parallel default(shared) private(hwthread, thread_id)
    {
        auto nthreads = omp_get_num_threads();
#pragma omp for ordered
        for (int i = 0 ; i < nthreads ; i++)
        {
#pragma omp ordered
            {
                thread_id = omp_get_thread_num();
                hwthread = sched_getcpu();
                cpu_set_t mask;
                CPU_ZERO(&mask);
                auto msize = sizeof(mask);
                sched_getaffinity(0, msize, &mask);
                int nhwthr = CPU_COUNT(&mask);
                std::string tmpstr;
                for (int i = 0; i < ncpus ; i++) {
                    // which hwthreads are in the set?
                    if (CPU_ISSET(i, &mask)) {
                        if (tmpstr.size() > 0) { tmpstr = tmpstr + ","; }
                        tmpstr = tmpstr + std::to_string(i);
                    }
                }
                auto lwp = gettid();
                tids.insert(lwp);

                printf("MPI %03d - SEC %d - OMP %03d - HWT %03d - LWP %06ld - #HWT %03d - Set %s \n",
                        rank, section, thread_id, hwthread, lwp, nhwthr, tmpstr.c_str());
            }
        }
    }
    return 0;
}

