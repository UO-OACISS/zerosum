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

#include <string.h>
#include <dirent.h>
#include <signal.h>
#include "zerosum.h"
#include "utils.h"

namespace zerosum {

int ZeroSum::getpthreads() {
    std::string tmpstr;
    DIR *dp;
    struct dirent *ep;
    dp = opendir ("/proc/self/task");
    static bool verbose{getVerbose()};
    static bool deadlock{parseBool("ZS_DETECT_DEADLOCK",false)};
    static int deadlock_duration{parseInt("ZS_DEADLOCK_DURATION",5)};
    static int deadlock_detected_seconds = -1;
    if (dp != NULL)
    {
        size_t running = 0;
        while ((ep = readdir (dp)) != NULL) {
            if (strncmp(ep->d_name, ".", 1) == 0) continue;
            if (tmpstr.size() > 0) { tmpstr = tmpstr + ","; }
            tmpstr = tmpstr + ep->d_name;
            uint32_t lwp = atol(ep->d_name);
            int nhwthr = 0;
            std::string tmpstr2;
            std::vector<uint32_t> allowed_list_posix =
                getAffinityList(lwp, computeNode.ncpus, nhwthr, tmpstr2);
            // also want to read /proc/<pid>/task/<tid>/status!
            std::string filename = "/proc/self/task/";
            filename += ep->d_name;
            filename += "/stat";
            auto fields = getThreadStat(filename.c_str());
            if (isRunning(fields, ep->d_name)) { running++; }
            filename += "us";
            std::string allowed_string = getCpusAllowed(filename.c_str());
            std::vector<uint32_t> allowed_list = parseDiscreteValues(allowed_string);
            getThreadStatus(filename.c_str(), fields);
            //std::cout << filename << " : " << allowed_string << std::endl;
            //fields.insert(std::pair("step",std::to_string(step)));
            if (lwp == async_tid) {
                this->process.add(lwp, allowed_list, fields, software::ThreadType::ZeroSum);
            } else {
                this->process.add(lwp, allowed_list, fields);
            }
        }
        (void) closedir (dp);
        // if there is only one running thread (this one, belonging to ZS), be concerned...
        if (deadlock) {
            int real_running = running-1;
            if (verbose) {
                ZeroSum::getInstance().getLogfile() << real_running << " threads running" << std::endl;
            }
            if (real_running == 0) {
                static int period{parseInt("ZS_PERIOD", 1)};
                deadlock_detected_seconds++;
                if (verbose) {
                    ZeroSum::getInstance().getLogfile() << "All threads sleeping for " <<
                        deadlock_detected_seconds*period << " seconds...?" << std::endl;
                }
            } else {
                deadlock_detected_seconds = -1;
            }
            if (deadlock_detected_seconds >= deadlock_duration) {
                ZeroSum::getInstance().getLogfile() << "Deadlock detected! Aborting!" << std::endl;
                ZeroSum::getInstance().getLogfile() << "Thread " << gettid() << " signalling " << this->process.id << std::endl;
                finalizeLog();
                pthread_kill(this->process.id, SIGQUIT);
            }
        }
    }
    return 0;
}

} // namespace zerosum
