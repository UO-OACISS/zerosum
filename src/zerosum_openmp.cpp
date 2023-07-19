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
#ifdef ZEROSUM_USE_OPENMP
#include <omp-tools.h>
#endif // ZEROSUM_USE_OPENMP

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(...) fprintf( stderr, __VA_ARGS__ ); fflush(stderr);
#else
#define DEBUG_PRINT(...) do{ } while ( false )
#endif


namespace zerosum {

void ZeroSum::getopenmp(void) {
#ifndef ZEROSUM_USE_OPENMP
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
#endif
}

#ifdef ZEROSUM_USE_OPENMP
extern "C" {

/* Function pointers.  These are all queried from the runtime during
 *  * ompt_initialize() */
static ompt_set_callback_t ompt_set_callback = nullptr;
static ompt_finalize_tool_t ompt_finalize_tool = nullptr;


/* Event #1, thread begin */
void zerosum_thread_begin(
    ompt_thread_t thread_type,   /* type of thread */
    ompt_data_t *thread_data     /* data of thread */)
{
    UNUSED(thread_data);
    switch (thread_type) {
        case ompt_thread_initial:
            DEBUG_PRINT("New OpenMP Initial Thread\n");
            break;
        case ompt_thread_worker:
            DEBUG_PRINT("New OpenMP Worker Thread\n");
            break;
        case ompt_thread_other:
            DEBUG_PRINT("New OpenMP Other Thread\n");
            break;
        case ompt_thread_unknown:
        default:
            DEBUG_PRINT("New OpenMP Unknown Thread\n");
    }
    int nhwthr = 0;
    std::string tmpstr;
    auto lwp = gettid();
    std::vector<uint32_t> allowed_list =
        getAffinityList(lwp, ZeroSum::getInstance().getComputeNode().ncpus, nhwthr, tmpstr);
    // also want to read /proc/<pid>/task/<tid>/status!
    std::string filename = "/proc/self/task/";
    filename += std::to_string(lwp);
    filename += "/stat";
    auto fields = getThreadStat(filename.c_str());
    filename += "us";
    getThreadStatus(filename.c_str(), fields);
    //fields.insert(std::pair("step",std::to_string(step)));
    ZeroSum::getInstance().getProcess().add(lwp, allowed_list, fields, software::ThreadType::OpenMP);
}

// This function is for checking that the function registration worked.
int zerosum_ompt_register(ompt_callbacks_t e, ompt_callback_t c ,
    const char * name) {
    UNUSED(name);
  DEBUG_PRINT("Registering OMPT callback %s...",name); fflush(stderr);
  ompt_set_result_t rc = ompt_set_callback(e, c);
  switch (rc) {
    case ompt_set_error:
        DEBUG_PRINT("\n\tFailed to register OMPT callback %s!\n",name);
        fflush(stderr);
        break;
    case ompt_set_never:
        DEBUG_PRINT("\n\tOMPT callback %s never supported by this runtime.\n",name);
        fflush(stderr);
        break;
    case ompt_set_impossible:
        DEBUG_PRINT("\n\tOMPT callback %s impossible from this runtime.\n",name);
        fflush(stderr);
        break;
    case ompt_set_sometimes:
        DEBUG_PRINT("\n\tOMPT callback %s sometimes supported by this runtime.\n",name);
        fflush(stderr);
        break;
    case ompt_set_sometimes_paired:
        DEBUG_PRINT("\n\tOMPT callback %s sometimes paired by this runtime.\n",name);
        fflush(stderr);
        break;
    case ompt_set_always:
    default:
        DEBUG_PRINT("success.\n");
  }
  return 0;
}


int ompt_initialize(ompt_function_lookup_t lookup, int initial_device_num,
    ompt_data_t* tool_data) {
    UNUSED(initial_device_num);
    UNUSED(tool_data);
    DEBUG_PRINT("Getting OMPT functions..."); fflush(stderr);
    //ompt_function_lookup = lookup;
    ompt_finalize_tool = (ompt_finalize_tool_t)
        lookup("ompt_finalize_tool");
    ompt_set_callback = (ompt_set_callback_t)
        lookup("ompt_set_callback");

    DEBUG_PRINT("success.\n");

    DEBUG_PRINT("Registering OMPT events...\n"); fflush(stderr);

    /* Mandatory events */

    // Event 1: thread begin
    zerosum_ompt_register(ompt_callback_thread_begin,
        (ompt_callback_t)&zerosum_thread_begin, "thread_begin");
    return 1;
}

void ompt_finalize(ompt_data_t* tool_data)
{
    UNUSED(tool_data);
    DEBUG_PRINT("OpenMP runtime is shutting down...\n");
}

/* According to the OpenMP 5.0 specification, this function needs to be
 * defined in the application address space.  The runtime will see it,
 * and run it. */
ompt_start_tool_result_t * ompt_start_tool(
    unsigned int omp_version, const char *runtime_version) {
    UNUSED(runtime_version); // in case we aren't printing debug messages
        DEBUG_PRINT("ZeroSum: OMPT Tool Start, version %d, '%s'\n",
            omp_version, runtime_version);
#if defined(_OPENMP)
        if (_OPENMP != omp_version) {
        DEBUG_PRINT("ZeroSum: WARNING! %d != %d (OpenMP Version used to compile ZeroSum)\n",
            omp_version, _OPENMP);
        }
#else
        UNUSED(omp_version); // in case we aren't printing debug messages
#endif
    static ompt_start_tool_result_t result;
    result.initialize = &ompt_initialize;
    result.finalize = &ompt_finalize;
    result.tool_data.value = 0L;
    result.tool_data.ptr = nullptr;
    return &result;
}

} // extern "C"
#endif // ZEROSUM_USE_OPENMP

}
