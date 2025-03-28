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

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <set>
#include <iomanip>
#include <string.h>
#include "zerosum.h"
#include <sycl/sycl.hpp>

namespace zerosum {

#define trycatch_string(_name, _thing) try { \
        fields.insert(std::pair(std::string(_name), d.get_info<_thing>())); \
    } catch (...) { \
        std::cerr << "Error reading SYCL device " << _name \
        << " for device " << index-1 << std::endl; \
    }

#define trycatch_size(_name, _thing) try { \
        fields.insert(std::pair(std::string(_name), \
            std::to_string(d.get_info<_thing>()))); \
    } catch (...) { \
        std::cerr << "Error reading SYCL device " << _name \
        << " for device " << index-1 << std::endl; \
    }

int ZeroSum::getgpu(void) {
    std::vector<std::map<std::string, std::string>> allfields;
    auto const& gpu_devices = sycl::device::get_devices(sycl::info::device_type::gpu);
    if (ZeroSum::getInstance().getRank() == 0 && getVerbose()) {
        std::cout << "Number of Root GPUs: " << gpu_devices.size() << std::endl;
    }
    size_t index{0};
    for (const auto& d : gpu_devices) {
        std::map<std::string, std::string> fields;
        fields.insert(std::pair(std::string("RT_GPU_ID"), std::to_string(index++)));
        trycatch_string("Name", sycl::info::device::name);
        trycatch_string("Vendor", sycl::info::device::vendor);
        trycatch_string("Driver Version", sycl::info::device::driver_version);
        trycatch_string("Version", sycl::info::device::version);
	/* Crashes on sunspot */
        //fields.insert(std::pair(std::string("Backend Version"), d.get_info<sycl::info::device::backend_version>()));
	/*
        size_t totalMemory = d.get_info<sycl::info::device::global_mem_size>();
        fields.insert(std::pair(std::string("TotalMem (bytes)"),
            std::to_string(totalMemory)));
	*/
        trycatch_size("TotalMem (bytes)", sycl::info::device::global_mem_size);
        trycatch_size("FreeMem (bytes)", sycl::ext::intel::info::device::free_memory);
        trycatch_size("Max Compute Units", sycl::info::device::max_compute_units);
        trycatch_size("Max Work Item Dimensions", sycl::info::device::max_work_item_dimensions);
        trycatch_size("Max Work Group Size", sycl::info::device::max_work_group_size);
        trycatch_size("Max Clock Frequency (MHz)", sycl::info::device::max_clock_frequency);
        trycatch_size("Max Memory Allocation Size (B)", sycl::info::device::max_mem_alloc_size);
        trycatch_size("Global Memory Cache Line Size (B)", sycl::info::device::global_mem_cache_line_size);
        trycatch_size("Global Memory Cache Size (B)", sycl::info::device::global_mem_cache_size);
        trycatch_size("Global Memory Size (B)", sycl::info::device::global_mem_size);
        trycatch_size("Local Memory Size (B)", sycl::info::device::local_mem_size);
        /* Get some intel-specific features */
        trycatch_size("Device ID", sycl::ext::intel::info::device::device_id);
	/* Crashes on sunspot? */
        trycatch_string("PCI Address", sycl::ext::intel::info::device::pci_address);
        trycatch_size("EU Count", sycl::ext::intel::info::device::gpu_eu_count);
        trycatch_size("EU SIMD Width", sycl::ext::intel::info::device::gpu_eu_simd_width);
        trycatch_size("Slices", sycl::ext::intel::info::device::gpu_slices);
        trycatch_size("Subslices per Slice", sycl::ext::intel::info::device::gpu_subslices_per_slice);
        trycatch_size("EU Count per Subslice", sycl::ext::intel::info::device::gpu_eu_count_per_subslice);
        trycatch_size("HW Threads per EU", sycl::ext::intel::info::device::gpu_hw_threads_per_eu);
	/* Crashes on sunspot? */
        //fields.insert(std::pair(std::string("Max Memory Bandwidth"),
            //std::to_string(d.get_info<sycl::ext::intel::info::device::max_mem_bandwidth>())));
	/* don't know how to convert uuid type */
        //fields.insert(std::pair(std::string("UUID"),
            //std::to_string(d.get_info<sycl::ext::intel::info::device::uuid>())));
        trycatch_size("Memory Clock Rate", sycl::ext::intel::info::device::memory_clock_rate);
        trycatch_size("Memory Bus Width", sycl::ext::intel::info::device::memory_bus_width);
        trycatch_size("Max Compute Queue Indices", sycl::ext::intel::info::device::max_compute_queue_indices);
        allfields.push_back(fields);
    }
    ZeroSum::getInstance().computeNode.addGpu(allfields);
/*
    // initialize level 0
    const ze_device_type_t type = ZE_DEVICE_TYPE_GPU;
    ZE_ERROR_CHECK(zeInit(0));
    // get the driver count
    uint32_t driverCount = 0;
    ze_driver_handle_t pDriver = nullptr;
    ze_device_handle_t pDevice = nullptr;
    ZE_ERROR_CHECK(zeDriverGet(&driverCount, nullptr));
    // get the drivers
    std::vector<ze_driver_handle_t> drivers( driverCount );
    ZE_ERROR_CHECK(zeDriverGet( &driverCount, drivers.data() ));
    for( uint32_t driver = 0; driver < driverCount; ++driver )
    {
        pDriver = drivers[driver];
        pDevice = findDevice( pDriver, type );
        if( pDevice ) {
            break;
        }
    }
    uint32_t pCount{0};
    zes_engine_handle_t tmp;
    // get the number of engine groups
    ZE_ERROR_CHECK(zesDeviceEnumEngineGroups(pDevice, &pCount, &tmp));
    std::vector<zes_engine_handle_t> engineHandles(pCount);
    ZE_ERROR_CHECK(zesDeviceEnumEngineGroups(pDevice, &pCount, engineHandles.data()));
    for ( auto e : engineHandles ) {
        zes_engine_properties_t pProperties;
        ZE_ERROR_CHECK(zesEngineGetProperties(e, &pProperties));
        std::cout << "engine type: " << pProperties.type << std::endl;
        std::cout << "on subdevice: " << pProperties.onSubdevice << std::endl;
        std::cout << "subdevice ID: " << pProperties.subdeviceId << std::endl;
    }
    */
    return 0;
}

}
