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
#include "level_zero/ze_api.h"
#include "level_zero/zes_api.h"
#include "sycl/ext/oneapi/backend/level_zero.hpp"

namespace zerosum {

    std::vector<sycl::device> initialize_sycl(void) {
        /* First things first - check if the environment is set correctly. */
        auto env = getenv("ZES_ENABLE_SYSMAN");
        if (env == nullptr || strncmp(env,"1",1) != 0) {
            std::cerr << "Error! Please set ZES_ENABLE_SYSMAN=1 in your environment for the monitoring to work correctly." << std::endl;
            //abort();
            std::cerr << "Exiting." << std::endl;
            exit(1);
        }
        auto const& gpu_devices = sycl::device::get_devices(sycl::info::device_type::gpu);
        if (ZeroSum::getInstance().getRank() == 0) {
                std::cout << "Number of Root GPUs: " << gpu_devices.size() << std::endl;
        }
#if 0
        sycl::queue Q(sycl::gpu_selector_v);
        std::cout << "Running on "
            << Q.get_device().get_info<sycl::info::device::name>()
            << "\n";
#endif
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
            /* do it with SYCL */
            size_t totalMemory = d.get_info<sycl::info::device::global_mem_size>();
            size_t freeMemory = d.get_info<sycl::ext::intel::info::device::free_memory>();
            fields.insert(std::pair(std::string("SYCL TotalMem (bytes)"),
                std::to_string(totalMemory)));
            fields.insert(std::pair(std::string("SYCL FreeMem (bytes)"),
                std::to_string(freeMemory)));
            /* do it with L0 */
#if 0
            zes_mem_state_t memory_props{
              ZES_STRUCTURE_TYPE_MEM_PROPERTIES,
            };

            // Get level-zero device handle
            auto ze_dev = ::sycl::get_native<::sycl::backend::ext_oneapi_level_zero>(d);

            uint32_t n_mem_modules = 1;
            std::vector<zes_mem_handle_t> module_list(n_mem_modules);
            zesDeviceEnumMemoryModules(ze_dev, &n_mem_modules, module_list.data());

            zesMemoryGetState(module_list[0], &memory_props);
            size_t total = memory_props.size;
            size_t free = memory_props.free;
            fields.insert(std::pair(std::string("L0 TotalMem (bytes)"),
                std::to_string(total)));
            fields.insert(std::pair(std::string("L0 FreeMem (bytes)"),
                std::to_string(free)));
#else
            // Get level-zero device handle
            ze_result_t status = ZE_RESULT_SUCCESS;
            auto ze_dev = ::sycl::get_native<::sycl::backend::ext_oneapi_level_zero>(d);
            uint32_t module_count = 0;
            status = zesDeviceEnumMemoryModules(ze_dev, &module_count, nullptr);
            if( module_count > 0 ) {
              std::vector<zes_mem_handle_t> module_list(module_count);
              std::vector<zes_mem_state_t> state_list(module_count);
          
              status = zesDeviceEnumMemoryModules(ze_dev, &module_count, module_list.data());
	      if (status == ZE_RESULT_SUCCESS) {
                  for (uint32_t i = 0; i < module_count; ++i) {
                    status = zesMemoryGetState(module_list[i], &(state_list[i]));
                    std::string prefix{"L0 M"};
                    prefix += std::to_string(i);
                    fields.insert(std::pair(prefix + std::string(" TotalMem (bytes)"),
                        std::to_string(state_list[i].size)));
                    fields.insert(std::pair(prefix + std::string(" FreeMem (bytes)"),
                        std::to_string(state_list[i].free)));
                  }
               }
            }
#endif

            allfields.push_back(fields);
        }
        computeNode.updateGPU(allfields);
        return 0;
    }
}

