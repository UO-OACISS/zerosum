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

#include "rocmsmi.h"
#include "hip/hip_runtime.h"

// Macro for checking errors in GPU API calls
#define gpuErrorCheck(call)                                                                  \
    do{                                                                                          \
        hipError_t gpuErr = call;                                                               \
        if(hipSuccess != gpuErr){                                                               \
            printf("GPU Error - %s:%d: '%s'\n", __FILE__, __LINE__, hipGetErrorString(gpuErr)); \
            exit(0);                                                                             \
        }                                                                                        \
    }while(0)

#define MICROWATTS 1.0e-6
#define VOLTAGE 1.0e-3  // scale mV to V
#define CELSIUS 1.0e-3  // scale mC to C
#define PERCENT 1.0e2  // scale fraction to %

namespace zerosum {

    std::vector<uint32_t> initialize_rocm_smi(void) {
        uint32_t deviceCount{0};
        RSMI_CALL(rsmi_init(0));
        RSMI_CALL(rsmi_num_monitor_devices(&deviceCount));
        rsmi_version_t version;
        RSMI_CALL(rsmi_version_get(&version));
        if (ZeroSum::getInstance().getRank() == 0) {
            std::cout << "RSMI Version "
                << version.major << "."
                << version.minor << "."
                << version.patch << " build "
                << version.build << ", Found "
                << deviceCount << " total devices" << std::endl;
        }
        // If HIP_VISIBLE_DEVICES is set, capture visible GPUs
        const char* gpu_visible_devices = getenv("ROCR_VISIBLE_DEVICES");
        std::vector<uint32_t> devices;
        if(gpu_visible_devices == NULL){
            for (uint32_t i = 0 ; i < deviceCount ; i++) {
                devices.push_back(i);
            }
        }
        else{
            if (ZeroSum::getInstance().getRank() == 0) {
                std::cout << "Visible devices: " << gpu_visible_devices << std::endl;
            }
            std::istringstream iss(gpu_visible_devices);
            std::string token;
            while (std::getline(iss, token, ',')) {
                devices.push_back(std::stol(token));
            }
        }
        return devices;
    }

    void finalize_rocm_smi(void) {
        RSMI_CALL(rsmi_shut_down());
    }

    int ZeroSum::getgpustatus(void) {
        // create a scoped variable to initialize/finalize
        static ScopedRSMI doInit;
        size_t value;
        std::vector<std::map<std::string, std::string>> allfields;

        // map d to the real device ID - ROCm can see all of them!
        for (int d : doInit.devices) {
            std::map<std::string, std::string> fields;
            std::string timerPrefix{"GPU: Device "};
            timerPrefix += std::to_string(d) + " ";
            /* Power, energy, temp, voltage */
            int64_t sensor_index{0};
            RSMI_CALL(rsmi_dev_power_ave_get(d, sensor_index, &value));
            fields.insert(std::pair(std::string("Power Average (W)"),
                std::to_string((double(value))*MICROWATTS)));
            float counter_resolution = 0;
            uint64_t timestamp;
            RSMI_CALL(rsmi_dev_energy_count_get(d, &value, &counter_resolution, &timestamp));
            fields.insert(std::pair(std::string("Energy Average (J)"),
                std::to_string((double(value))*MICROWATTS)));
            int64_t temperature;
            RSMI_CALL(rsmi_dev_temp_metric_get(d, sensor_index, RSMI_TEMP_CURRENT, &temperature));
            fields.insert(std::pair(std::string("Temperature (C)"),
                std::to_string((double(temperature))*CELSIUS)));
            RSMI_CALL(rsmi_dev_volt_metric_get(d, RSMI_VOLT_TYPE_VDDGFX, RSMI_VOLT_CURRENT, &temperature));
            fields.insert(std::pair(std::string("Voltage (mV)"),
                std::to_string((double(temperature)))));


            /* Memory */
            RSMI_CALL(rsmi_dev_memory_total_get(d, RSMI_MEM_TYPE_VRAM, &value));
            fields.insert(std::pair(std::string("Total VRAM Bytes"), std::to_string(value)));
            RSMI_CALL(rsmi_dev_memory_usage_get(d, RSMI_MEM_TYPE_VRAM, &value));
            fields.insert(std::pair(std::string("Used VRAM Bytes"), std::to_string(value)));
            RSMI_CALL(rsmi_dev_memory_total_get(d, RSMI_MEM_TYPE_VIS_VRAM, &value));
            fields.insert(std::pair(std::string("Total Visible VRAM Bytes"), std::to_string(value)));
            RSMI_CALL(rsmi_dev_memory_usage_get(d, RSMI_MEM_TYPE_VIS_VRAM, &value));
            fields.insert(std::pair(std::string("Used Visible VRAM Bytes"), std::to_string(value)));
            RSMI_CALL(rsmi_dev_memory_total_get(d, RSMI_MEM_TYPE_GTT, &value));
            fields.insert(std::pair(std::string("Total GTT Bytes"), std::to_string(value)));
            RSMI_CALL(rsmi_dev_memory_usage_get(d, RSMI_MEM_TYPE_GTT, &value));
            fields.insert(std::pair(std::string("Used GTT Bytes"), std::to_string(value)));

            /* utilization */
            uint32_t percent;
            RSMI_CALL(rsmi_dev_memory_busy_percent_get(d, &percent));
            fields.insert(std::pair(std::string("Memory Busy %"), std::to_string(percent)));
            RSMI_CALL(rsmi_dev_busy_percent_get (d, &percent));
            fields.insert(std::pair(std::string("Device Busy %"), std::to_string(percent)));
            rsmi_utilization_counter_t utilization_counters[2];
            utilization_counters[0].type = RSMI_COARSE_GRAIN_GFX_ACTIVITY;
            utilization_counters[1].type = RSMI_COARSE_GRAIN_MEM_ACTIVITY;
            RSMI_CALL(rsmi_utilization_count_get (d, utilization_counters, 2, &timestamp));
            fields.insert(std::pair(std::string("GFX Activity %"), std::to_string(utilization_counters[0].value)));
            fields.insert(std::pair(std::string("Memory Activity %"), std::to_string(utilization_counters[1].value)));

            /* Metrics */
            rsmi_gpu_metrics_t pgpu_metrics;
            memset(&pgpu_metrics, 0, sizeof(rsmi_gpu_metrics_t));
            RSMI_CALL(rsmi_dev_gpu_metrics_info_get (d, &pgpu_metrics));
            fields.insert(std::pair(std::string("Clock Frequency, GLX (MHz)"), std::to_string(pgpu_metrics.current_gfxclk)));
            fields.insert(std::pair(std::string("Clock Frequency, SOC (MHz)"), std::to_string(pgpu_metrics.current_socclk)));
            fields.insert(std::pair(std::string("GFX Activity"), std::to_string(pgpu_metrics.average_gfx_activity)));
            fields.insert(std::pair(std::string("Memory Controller Activity"), std::to_string(pgpu_metrics.average_umc_activity)));
            fields.insert(std::pair(std::string("UVD|VCN Activity"), std::to_string(pgpu_metrics.average_mm_activity)));
            fields.insert(std::pair(std::string("Throttle Status"), std::to_string(pgpu_metrics.throttle_status)));

            allfields.push_back(fields);
        }
        computeNode.updateGPU(allfields);
        return 0;
    }
}

