#include <sycl/sycl.hpp>
#include "level_zero/ze_api.h"
#include "level_zero/zes_api.h"
#include "sycl/ext/oneapi/backend/level_zero.hpp"

constexpr auto billion=1024*1024*1024;

int main() {
    auto const& gpu_devices = sycl::device::get_devices(sycl::info::device_type::gpu);
    std::cout << "Number of Root GPUs: " << gpu_devices.size() << std::endl;

    for(const auto& d : gpu_devices) {
        /* go through sycl */
        std::cout << "SYCL GPU-ID: " << d.get_info<sycl::info::device::name>() << std::endl;
        size_t totalMemory = d.get_info<sycl::info::device::global_mem_size>();
        size_t freeMemory = totalMemory;
        try {
            freeMemory = d.get_info<sycl::ext::intel::info::device::free_memory>();
        } catch (...) {
            std::cerr << "Error reading intel device free memory." << std::endl;
        }
        std::cout << "SYCL TotalMem (GB): " << totalMemory / billion
            << ", FreeMem (GB): " << freeMemory / billion << std::endl;

        /* go through level0 */
        ze_result_t status = ZE_RESULT_SUCCESS;
        try {
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
                        std::string prefix{"Level0 Module "};
                        prefix += std::to_string(i);
                        std::cout << prefix << " TotalMem (GB): "
                            << state_list[i].size / billion
                            << ", FreeMem (GB): "
                            << state_list[i].free / billion << std::endl;
                    }
                }
            }
        } catch (...) {
            std::cerr << "Error getting native intel device." << std::endl;
        }
        std::cout << std::endl;
    }
    return 0;
}
