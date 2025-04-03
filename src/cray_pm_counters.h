/*
# MIT License
#
# Copyright (c) 2023-2025 University of Oregon, Kevin Huck
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
*/

#pragma once

#include <string>
#include <map>
#include <set>
#include <tuple>
#include <cstdint>

namespace zerosum {

typedef std::tuple<std::string, uint64_t, bool> cray_tuple;
class cray_pm_counters {
    public:
        cray_pm_counters(void);
        ~cray_pm_counters(void);
        std::map<std::string,std::string> read_counters(void);
        uint64_t get_unitless(std::string name);
        std::pair<uint64_t,std::string> get_with_unit(std::string name);
    private:
        bool supported;
        const std::string location{"/sys/cray/pm_counters/"};
        const std::string freshness{"freshness"};
        const std::string generation{"generation"};
        const std::set<std::string> skip = { "startup", "version", "raw_scan_hz", "generation", "freshness" };
        // each metric has a name, full name with unit, and previous value
        std::map<std::string, cray_tuple> previous;
        uint64_t previous_generation;
        uint64_t previous_freshness;
};

}

