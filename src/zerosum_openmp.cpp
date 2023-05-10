#include "zerosum.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <set>
#include <iomanip>
#include <iomanip>
#include <string.h>
#include <omp.h>
#include <sys/types.h>
#include <dirent.h>
#include <set>
#include "omp.h"
#include "utils.h"

namespace zerosum {

void ZeroSum::getopenmp() {
    int hwthread;
    int thread_id = 0;
    std::string outdata;

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
                int nhwthr = 0;
                std::string tmpstr;
                auto lwp = gettid();
                std::vector<uint32_t> allowed_list =
                    getAffinityList(lwp, computeNode.ncpus, nhwthr, tmpstr);
                // also want to read /proc/<pid>/task/<tid>/status!
                std::string filename = "/proc/self/task/";
                filename += std::to_string(lwp);
                filename += "/stat";
                auto fields = getThreadStat(filename.c_str());
                filename += "us";
                getThreadStatus(filename.c_str(), fields);
                fields.insert(std::pair("step",std::to_string(step)));
                this->process.add(lwp, allowed_list, fields, software::ThreadType::OpenMP);
            }
        }
    }
}

}
