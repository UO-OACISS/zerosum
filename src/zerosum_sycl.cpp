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

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <set>
#include <iomanip>
#include <string.h>
#include "zerosum.h"
#include <sycl/sycl.hpp>


namespace zerosum {

int ZeroSum::getgpu(void) {
    std::vector<std::map<std::string, std::string>> allfields;
    auto const& gpu_devices = sycl::device::get_devices(sycl::info::device_type::gpu);
    if (ZeroSum::getInstance().getRank() == 0) {
        std::cout << "Number of Root GPUs: " << gpu_devices.size() << std::endl;
    }
    size_t index{0};
    for (const auto& d : gpu_devices) {
        std::map<std::string, std::string> fields;
        size_t totalMemory = d.get_info<sycl::info::device::global_mem_size>();
        size_t freeMemory = d.get_info<sycl::ext::intel::info::device::free_memory>();
        if (ZeroSum::getInstance().getRank() == 0) {
            std::cout << "(root-dev) GPU-ID: " << d.get_info<sycl::info::device::name>() << std::endl;
            std::cout << "(root-dev) TotalMem (bytes): " << totalMemory << ", FreeMem (bytes): " << freeMemory << std::endl;
        }
        fields.insert(std::pair(std::string("Name"), d.get_info<sycl::info::device::name>()));
        fields.insert(std::pair(std::string("RT_GPU_ID"), std::to_string(index++)));
        fields.insert(std::pair(std::string("TotalMem (bytes)"),
            std::to_string(totalMemory)));
        fields.insert(std::pair(std::string("FreeMem (bytes)"),
            std::to_string(freeMemory)));

        allfields.push_back(fields);
    }
    ZeroSum::getInstance().computeNode.addGpu(allfields);
    return 0;
}

}
