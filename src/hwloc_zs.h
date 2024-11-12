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

#pragma once

#include "hwloc.h"
#include <map>
#include <iostream>
#include <fstream>
#include <string>

namespace zerosum {

namespace software {
    class Process;
}

class ScopedHWLOC {
private:
    hwloc_topology_t topo;
    hwloc_obj_t root;
    void traverse(hwloc_obj_t obj, size_t indent=0);
public:
    void traverse(void) {
        //std::cout << "HWLOC Node topology:" << std::endl;
        traverse(root);
    }
    void buildJSON(void);
    std::pair<std::string, uint32_t> buildJSON(hwloc_obj_t obj, int32_t depth);
    static std::map<size_t, size_t>& getHWTMap();
    static void validate_hwloc(int shmrank);
    ScopedHWLOC(void) {
        hwloc_topology_init(&topo);
        unsigned long flags = hwloc_topology_get_flags(topo);
        //std::cout << "Flags before: " << flags << std::endl;
        flags = flags | HWLOC_TOPOLOGY_FLAG_INCLUDE_DISALLOWED;
        //std::cout << "Flags after: " << flags << std::endl;
        hwloc_topology_set_flags(topo, flags);
        //hwloc_topology_set_all_types_filter(topo, HWLOC_TYPE_FILTER_KEEP_ALL);
        hwloc_topology_set_io_types_filter(topo, HWLOC_TYPE_FILTER_KEEP_IMPORTANT);
	    hwloc_topology_load(topo);
        root = hwloc_get_root_obj(topo);
    }
    ~ScopedHWLOC(void) {
        hwloc_topology_destroy(topo);
    }

};

}
