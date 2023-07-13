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
#include "utils.h"

namespace zerosum {

void ZeroSum::getopenmp() {
#pragma omp parallel default(shared)
    {
        int nthreads = omp_get_num_threads();
#pragma omp for ordered
        for (int i = 0 ; i < nthreads ; i++)
        {
#pragma omp ordered
            {
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
                //fields.insert(std::pair("step",std::to_string(step)));
                this->process.add(lwp, allowed_list, fields, software::ThreadType::OpenMP);
            }
        }
    }
}

}
