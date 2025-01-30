/*
 * Copyright (c) 2014-2025 Kevin Huck
 * Copyright (c) 2014-2025 University of Oregon
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <string>
#include <map>
#include <set>
#include <tuple>

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

