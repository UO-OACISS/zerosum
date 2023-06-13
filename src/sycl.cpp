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
#include "sycl_zs.h"
#include <climits>

namespace zerosum {

    std::vector<sycl::device> initialize_sycl(void) {
        uint32_t deviceCount{0};
	auto const& gpu_devices = sycl::device::get_devices(sycl::info::device_type::gpu);
        std::cout << "Number of Root GPUs: " << gpu_devices.size() << std::endl;

        std::vector<sycl::device> devices;
        for (const auto& d : gpu_devices) {
            devices.push_back(d);
        }
        return devices;
    }

    void finalize_sycl(void) {
        // nothing to do
    }

    int ZeroSum::getgpustatus(void) {
        // create a scoped variable to initialize/finalize
        static ScopedSYCL doInit;
        std::vector<std::map<std::string, std::string>> allfields;
        for (sycl::device d : doInit.devices) {
            std::map<std::string, std::string> fields;

            std::cout << "(root-dev) GPU-ID: " << d.get_info<sycl::info::device::name>() << std::endl;
            size_t totalMemory = d.get_info<sycl::info::device::global_mem_size>();
            size_t freeMemory = d.get_info<sycl::ext::intel::info::device::free_memory>();
            std::cout << "(root-dev) TotalMem (bytes): " << totalMemory << ", FreeMem (bytes): " << freeMemory << std::endl;

            fields.insert(std::pair(std::string("TotalMem (bytes):"),
                std::to_string(totalMemory)));
            fields.insert(std::pair(std::string("FreeMem (bytes):"),
                std::to_string(freeMemory)));

            allfields.push_back(fields);
        }
        computeNode.updateGPU(allfields);
        return 0;
    }
}

