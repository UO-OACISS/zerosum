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
#include "zerosum.h"
#include "utils.h"

namespace zerosum {

int ZeroSum::getpthreads() {
    std::string tmpstr;
    DIR *dp;
    struct dirent *ep;
    dp = opendir ("/proc/self/task");
    if (dp != NULL)
    {
        while ((ep = readdir (dp)) != NULL) {
            if (strncmp(ep->d_name, ".", 1) == 0) continue;
            if (tmpstr.size() > 0) { tmpstr = tmpstr + ","; }
//fields.insert(std::pair("step",std::to_string(step)));
            if (lwp == async_tid) {
                this->process.add(lwp, allowed_list, fields, software::ThreadType::ZeroSum);
            } else {
                this->process.add(lwp, allowed_list, fields);
            }
        }
        (void) closedir (dp);
    }
    return 0;
}

} // namespace zerosum
