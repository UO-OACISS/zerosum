/**********************************************************
  "Hello World"-type program to test different srun layouts.

  Written by Tom Papatheodore
 **********************************************************/

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
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)
#include "zerosum.h"

int ZeroSum::getpthreads(const int rank, const int section, const int ncpus, std::set<long>& tids) {
    std::string tmpstr;
    DIR *dp;
    struct dirent *ep;
    dp = opendir ("/proc/self/task");
    if (dp != NULL)
    {
        while ((ep = readdir (dp)) != NULL) {
            if (strncmp(ep->d_name, ".", 1) == 0) continue;
            if (tmpstr.size() > 0) { tmpstr = tmpstr + ","; }
            tmpstr = tmpstr + ep->d_name;
            long lwp = atol(ep->d_name);
            // also want to read /proc/<pid>/task/<tid>/status!
            // see https://stackoverflow.com/questions/8032372/how-can-i-see-which-cpu-core-a-thread-is-running-in#:~:text=To%20get%20the%20information%20you,if%20it%27s%20not%20currently%20running.&text=Not%20currently%20running.,-Last%20ran%20on
            if (!tids.count(lwp)) {
                cpu_set_t mask;
                CPU_ZERO(&mask);
                auto msize = sizeof(mask);
                sched_getaffinity(lwp, msize, &mask);
                int nhwthr = CPU_COUNT(&mask);
                std::string tmpstr2;
                for (int i = 0; i < ncpus ; i++) {
                    // which hwthreads are in the set?
                    if (CPU_ISSET(i, &mask)) {
                        if (tmpstr2.size() > 0) { tmpstr2 = tmpstr2 + ","; }
                        tmpstr2 = tmpstr2 + std::to_string(i);
                    }
                }
                char buffer[1025];
                snprintf(buffer, 1024,
                    "MPI %03d - STEP %03d - SEC %d - LWP %06ld - #HWT %03d - Set [%s]",
                    rank, step, section, lwp, nhwthr, tmpstr2.c_str());
                logfile << buffer << std::endl;

            }
        }
        (void) closedir (dp);
    }
    char buffer[1025];
    snprintf(buffer, 1024,
        "MPI %03d - STEP %03d - SEC %d - LWPS [%s]", rank, step, section, tmpstr.c_str());
    logfile << buffer << std::endl;


    return 0;
}
