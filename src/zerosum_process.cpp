/*
 * MIT License
 *
 * Copyright (c) 2023-2025 University of Oregon, Kevin Huck
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
#include "utils.h"
#include <dirent.h>
#include <set>
#include <vector>
#include <bits/stdc++.h>
#include <string.h>

namespace zerosum {

/*
void ZeroSum::getProcStatus(const int rank, const int size) {
    std::string allowed_string = getCpusAllowed("/proc/self/status");
    std::vector<uint32_t> allowed_list = parseDiscreteValues(allowed_string);
    this->process = software::Process(getpid(), rank, size, allowed_list);
    this->process.hwthreads_raw = allowed_string;
    this->process.computeNode = this->computeNode;
    logfile << this->process.toString() << std::endl;
    return;
}
*/

/* This function will find any other processes that are running on our assigned resources */
int ZeroSum::getOtherProcesses(void) {
    std::string tmpstr;
    DIR *dp;
    struct dirent *ep;
    dp = opendir ("/proc");
    if (dp != NULL)
    {
        while ((ep = readdir (dp)) != NULL) {
            // skip . and ..
            if (strncmp(ep->d_name, ".", 1) == 0) continue;
            // skip any non-numeric names
            std::string pid{ep->d_name};
            if (!all_of(pid.begin(), pid.end(), ::isdigit)) { continue; }
            // skip ourselves
            if (stol(pid) == process.id) { continue; }
            if (tmpstr.size() > 0) { tmpstr = tmpstr + ","; }
            std::string statfile{"/proc/" + pid + "/status"};
            std::string allowed_string = getCpusAllowed(statfile.c_str());
            std::vector<uint32_t> allowed_list = parseDiscreteValues(allowed_string);
            //this->process = software::Process(getpid(), rank, size, allowed_list);
            bool overlap = false;
            for (auto hwt : allowed_list) {
                if (process.hwthreads.count(hwt) > 0) {
                    overlap = true;
                    break;
                }
            }
            if (overlap) {
                std::string filename = "/proc/" + pid + "/stat";
                auto fields = getThreadStat(filename.c_str());
                filename = "/proc/" + pid + "/status";
                getThreadStatus(filename.c_str(), fields);
                otherProcesses.push_back(software::Process(stol(pid), 0, 1, fields, allowed_list));
            }
        }
        (void) closedir (dp);
    }
    return 0;
}


}
