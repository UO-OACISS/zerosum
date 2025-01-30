/*
 * Copyright (c) 2014-2025 Kevin Huck
 * Copyright (c) 2014-2025 University of Oregon
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include <sys/types.h>
#include <dirent.h>
#include "cray_pm_counters.h"
#include <string>
#include <iostream>
#include <sstream>
#include <string.h>

using namespace std;

namespace zerosum {

cray_pm_counters::cray_pm_counters() : supported(false)
{
    struct dirent *entry = nullptr;
    DIR * directory_ptr = opendir(location.c_str());
    if (directory_ptr == NULL) return;
    // get a list of metrics available
    while ((entry = readdir(directory_ptr))) {
        if (entry->d_type == DT_DIR) continue;
        // get the metric name
        std::string name(entry->d_name);
        // if it is a cap, skip it
        if (strstr(entry->d_name,"_cap") != 0) continue;
        if (skip.count(name) != 0) continue;
        // read the initial value, save units and full name
        uint64_t tmpint;
        std::string tmpstr;
        std::string filename = location + name;
        FILE * fp = fopen(filename.c_str(), "r");
        if (fp != NULL) {
            char *line = NULL;
            size_t len = 0;
            ssize_t nread;
            while ((nread = getline(&line, &len, fp)) != -1) {
                if (nread == -1) break;
                std::istringstream iline{line};
                while (iline >> tmpint >> tmpstr) {
                    bool monotonic{false};
                    std::string fullname = name + std::string(" (") +
                        tmpstr + std::string(")");
                    if (tmpstr.compare("J") == 0) { monotonic = true; }
                    previous.insert(
                        std::pair<std::string,cray_tuple>(
                            name, std::make_tuple(
                                fullname, tmpint, monotonic)));
                    break;
                }
            }
        }
        fclose(fp);
    }
    supported = true;
    closedir(directory_ptr);
}

cray_pm_counters::~cray_pm_counters()
{
}

std::map<std::string,std::string> cray_pm_counters::read_counters()
{
    std::map<std::string,std::string> fields;
    if (!supported) return fields;
    // iterate over the tuples, with a reference because we want to update them
    for (auto& p : previous) {
        std::string fullname;
        uint64_t prev;
        bool monotonic;
        // tie will give us references to these values...
        std::tie(fullname, prev, monotonic) = p.second;
        uint64_t tmpint;
        std::string tmpstr;
        std::string filename = location + p.first;
        FILE * fp = fopen(filename.c_str(), "r");
        if (fp != NULL) {
            char *line = NULL;
            size_t len = 0;
            ssize_t nread;
            while ((nread = getline(&line, &len, fp)) != -1) {
                if (nread == -1) break;
                std::istringstream iline{line};
                while (iline >> tmpint >> tmpstr) {
                    auto current = tmpint;
                    if (monotonic) {
                        current = current - prev;
                        std::get<1>(p.second) = tmpint;
                    }
                    fields.insert(
                        std::pair<std::string,std::string>(
                            fullname, std::to_string(current)));
                    break;
                }
            }
        }
        fclose(fp);
    }
    return fields;
}

}
