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

#include <sys/types.h>
#include <dirent.h>
#include "cray_pm_counters.h"
#include <string>
#include <iostream>
#include <sstream>
#include <string.h>

using namespace std;

namespace zerosum {

uint64_t cray_pm_counters::get_unitless(std::string name) {
    uint64_t tmpint{0};
    std::string filename = location + name;
    FILE * fp = fopen(filename.c_str(), "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, fp)) != -1) {
        if (nread == -1) break;
        std::istringstream iline{line};
        while (iline >> tmpint) {
            return tmpint;
            break;
        }
    }
    fclose(fp);
    return tmpint;
}

std::pair<uint64_t,std::string> cray_pm_counters::get_with_unit(std::string name) {
    uint64_t tmpint{0};
    std::string tmpstr;
    std::string filename = location + name;
    FILE * fp = fopen(filename.c_str(), "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, fp)) != -1) {
        if (nread == -1) break;
        std::istringstream iline{line};
        while (iline >> tmpint >> tmpstr) {
            break;
        }
    }
    fclose(fp);
    return std::make_pair(tmpint, tmpstr);
}

cray_pm_counters::cray_pm_counters() : supported(false),
    previous_generation(0), previous_freshness(0)
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
        auto data = get_with_unit(name);
        auto tmpint = std::get<uint64_t>(data);
        auto tmpstr = std::get<std::string>(data);
        bool monotonic{false};
        std::string fullname = std::string("cray_pm ") + name +
            std::string(" (") + tmpstr + std::string(")");
        if (tmpstr.compare("J") == 0) { monotonic = true; }
        previous.insert(
            std::pair<std::string,cray_tuple>(
                name, std::make_tuple(
                    fullname, tmpint, monotonic)));
    }
    previous_freshness = get_unitless("freshness");
    previous_generation = get_unitless("generation");
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

    // confirm we have good values
    uint64_t new_freshness = get_unitless("freshness");
    //fields.insert( std::pair<std::string,std::string>(
        //"freshness", std::to_string(new_freshness)));
    bool valid{new_freshness > previous_freshness};
    fields.insert( std::pair<std::string,std::string>(
        "cray_pm valid", std::to_string(valid)));
    previous_freshness = new_freshness;

    // confirm we haven't changed power caps
    uint64_t new_generation = get_unitless("generation");
    bool changed{new_generation > previous_generation};
    fields.insert( std::pair<std::string,std::string>(
        "cray_pm power cap changed", std::to_string(changed)));
    previous_generation = new_generation;

    // iterate over the tuples, with a reference because we want to update them
    for (auto& p : previous) {
        std::string fullname;
        uint64_t prev;
        bool monotonic;
        // tie will give us references to these values...
        std::tie(fullname, prev, monotonic) = p.second;
        auto data = get_with_unit(p.first);
        auto tmpint = std::get<uint64_t>(data);
        auto current = tmpint;
        if (monotonic) {
            current = current - prev;
            std::get<1>(p.second) = tmpint;
        }
        fields.insert(
            std::pair<std::string,std::string>(
                fullname, std::to_string(current)));
    }
    return fields;
}

}
