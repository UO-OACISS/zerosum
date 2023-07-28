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
    if (ZeroSum::getInstance().getRank() == 0 && getVerbose()) {
        std::cout << "Number of Root GPUs: " << gpu_devices.size() << std::endl;
    }
    size_t index{0};
    for (const auto& d : gpu_devices) {
        std::map<std::string, std::string> fields;
        try {
        fields.insert(std::pair(std::string("Name"), d.get_info<sycl::info::device::name>()));
        fields.insert(std::pair(std::string("Vendor"), d.get_info<sycl::info::device::vendor>()));
        fields.insert(std::pair(std::string("Driver Version"), d.get_info<sycl::info::device::driver_version>()));
        fields.insert(std::pair(std::string("Version"), d.get_info<sycl::info::device::version>()));
	/* Crashes on sunspot */
        //fields.insert(std::pair(std::string("Backend Version"), d.get_info<sycl::info::device::backend_version>()));
        fields.insert(std::pair(std::string("RT_GPU_ID"), std::to_string(index++)));
        size_t totalMemory = d.get_info<sycl::info::device::global_mem_size>();
        fields.insert(std::pair(std::string("TotalMem (bytes)"),
            std::to_string(totalMemory)));
        size_t freeMemory = d.get_info<sycl::ext::intel::info::device::free_memory>();
        fields.insert(std::pair(std::string("FreeMem (bytes)"),
            std::to_string(freeMemory)));
        fields.insert(std::pair(std::string("Max Compute Units"),
            std::to_string(d.get_info<sycl::info::device::max_compute_units>())));
        fields.insert(std::pair(std::string("Max Work Item Dimensions"),
            std::to_string(d.get_info<sycl::info::device::max_work_item_dimensions>())));
        fields.insert(std::pair(std::string("Max Work Group Size"),
            std::to_string(d.get_info<sycl::info::device::max_work_group_size>())));
        fields.insert(std::pair(std::string("Max Clock Frequency (MHz)"),
            std::to_string(d.get_info<sycl::info::device::max_clock_frequency>())));
        fields.insert(std::pair(std::string("Max Memory Allocation Size (B)"),
            std::to_string(d.get_info<sycl::info::device::max_mem_alloc_size>())));
        fields.insert(std::pair(std::string("Global Memory Cache Line Size (B)"),
            std::to_string(d.get_info<sycl::info::device::global_mem_cache_line_size>())));
        fields.insert(std::pair(std::string("Global Memory Cache Size (B)"),
            std::to_string(d.get_info<sycl::info::device::global_mem_cache_size>())));
        fields.insert(std::pair(std::string("Global Memory Size (B)"),
            std::to_string(d.get_info<sycl::info::device::global_mem_size>())));
        fields.insert(std::pair(std::string("Local Memory Size (B)"),
            std::to_string(d.get_info<sycl::info::device::local_mem_size>())));
        /* Get some intel-specific features */
        fields.insert(std::pair(std::string("Device ID"),
            std::to_string(d.get_info<sycl::ext::intel::info::device::device_id>())));
	/* Crashes on sunspot? */
        fields.insert(std::pair(std::string("PCI Address"), d.get_info<sycl::ext::intel::info::device::pci_address>()));
        fields.insert(std::pair(std::string("EU Count"),
            std::to_string(d.get_info<sycl::ext::intel::info::device::gpu_eu_count>())));
        fields.insert(std::pair(std::string("EU SIMD Width"),
            std::to_string(d.get_info<sycl::ext::intel::info::device::gpu_eu_simd_width>())));
        fields.insert(std::pair(std::string("Slices"),
            std::to_string(d.get_info<sycl::ext::intel::info::device::gpu_slices>())));
        fields.insert(std::pair(std::string("Subslices per Slice"),
            std::to_string(d.get_info<sycl::ext::intel::info::device::gpu_subslices_per_slice>())));
        fields.insert(std::pair(std::string("EU Count per Subslice"),
            std::to_string(d.get_info<sycl::ext::intel::info::device::gpu_eu_count_per_subslice>())));
        fields.insert(std::pair(std::string("HW Threads per EU"),
            std::to_string(d.get_info<sycl::ext::intel::info::device::gpu_hw_threads_per_eu>())));
	/* Crashes on sunspot? */
        //fields.insert(std::pair(std::string("Max Memory Bandwidth"),
            //std::to_string(d.get_info<sycl::ext::intel::info::device::max_mem_bandwidth>())));
	/* don't know how to convert uuid type */
        //fields.insert(std::pair(std::string("UUID"),
            //std::to_string(d.get_info<sycl::ext::intel::info::device::uuid>())));
        fields.insert(std::pair(std::string("Memory Clock Rate"),
            std::to_string(d.get_info<sycl::ext::intel::info::device::memory_clock_rate>())));
        fields.insert(std::pair(std::string("Memory Bus Width"),
            std::to_string(d.get_info<sycl::ext::intel::info::device::memory_bus_width>())));
        fields.insert(std::pair(std::string("Max Compute Queue Indices"),
            std::to_string(d.get_info<sycl::ext::intel::info::device::max_compute_queue_indices>())));
        allfields.push_back(fields);
        } catch (...) {
            std::cerr << "Error reading SYCL device info for device " << index-1 << std::endl;
        }
    }
    ZeroSum::getInstance().computeNode.addGpu(allfields);
    return 0;
}

}
