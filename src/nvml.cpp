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
#include "nvml_zs.h"
#include <climits>

namespace zerosum {

    std::vector<nvmlDevice_t> initialize_nvml(void) {
        uint32_t deviceCount{0};
        NVML_CALL(nvmlInit_v2());
        NVML_CALL(nvmlDeviceGetCount_v2(&deviceCount));
        char version_name[NVML_SYSTEM_NVML_VERSION_BUFFER_SIZE];
        NVML_CALL(nvmlSystemGetNVMLVersion(version_name, NVML_SYSTEM_NVML_VERSION_BUFFER_SIZE));
        if (ZeroSum::getInstance().getRank() == 0) {
            std::cout << "NVML Version "
                << version_name << ", Found "
                << deviceCount << " total devices" << std::endl;
        }
        // If HIP_VISIBLE_DEVICES is set, capture visible GPUs
        const char* gpu_visible_devices = getenv("CUDA_VISIBLE_DEVICES");
        std::vector<nvmlDevice_t> devices;
        if(gpu_visible_devices == NULL){
            for (uint32_t i = 0 ; i < deviceCount ; i++) {
                nvmlDevice_t device;
                NVML_CALL(nvmlDeviceGetHandleByIndex_v2(i, &device));
                devices.push_back(device);
            }
        }
        else{
            if (ZeroSum::getInstance().getRank() == 0) {
                std::cout << "Visible devices: " << gpu_visible_devices << std::endl;
            }
            std::istringstream iss(gpu_visible_devices);
            std::string token;
            while (std::getline(iss, token, ',')) {
                nvmlDevice_t device;
                NVML_CALL(nvmlDeviceGetHandleByIndex_v2(std::stol(token), &device));
                devices.push_back(device);
            }
        }
        return devices;
    }

    void finalize_nvml(void) {
        NVML_CALL(nvmlShutdown());
    }

    double convertValue(nvmlFieldValue_t &value) {
        if (value.valueType == NVML_VALUE_TYPE_DOUBLE) {
            return value.value.dVal;
        } else if (value.valueType == NVML_VALUE_TYPE_UNSIGNED_INT) {
            return (double)(value.value.uiVal);
        } else if (value.valueType == NVML_VALUE_TYPE_UNSIGNED_LONG) {
            return (double)(value.value.ulVal);
        } else if (value.valueType == NVML_VALUE_TYPE_UNSIGNED_LONG_LONG) {
            return (double)(value.value.ullVal);
        } else if (value.valueType == NVML_VALUE_TYPE_SIGNED_LONG_LONG) {
            return (double)(value.value.sllVal);
        }
        return 0.0;
    }

    int ZeroSum::getgpustatus(void) {
        // create a scoped variable to initialize/finalize
        static ScopedNVML doInit;
        std::vector<std::map<std::string, std::string>> allfields;
        for (nvmlDevice_t dev : doInit.devices) {
            std::map<std::string, std::string> fields;

            /* Get overall utilization percentages */
            nvmlUtilization_t utilization;
            NVML_CALL(nvmlDeviceGetUtilizationRates(dev, &utilization));
            fields.insert(std::pair(std::string("Utilization %"),
                std::to_string(utilization.gpu)));
            fields.insert(std::pair(std::string("Memory Utilization %"),
                std::to_string(utilization.memory)));

            /* Get memory bytes allocated */
            nvmlMemory_t memory;
            NVML_CALL(nvmlDeviceGetMemoryInfo(dev, &memory));
            fields.insert(std::pair(std::string("Memory Total (GB)"),
                std::to_string(memory.total * BILLIONTH)));
            fields.insert(std::pair(std::string("Memory Free (GB)"),
                std::to_string(memory.free * BILLIONTH)));
            fields.insert(std::pair(std::string("Memory Used (GB)"),
                std::to_string(memory.used * BILLIONTH)));

            /* Get clock settings */
            uint32_t clock = 0;
            NVML_CALL(nvmlDeviceGetClock(dev, NVML_CLOCK_SM,
                    NVML_CLOCK_ID_CURRENT, &clock));
            fields.insert(std::pair(std::string("Clock SM (MHz)"),
                std::to_string(clock)));
            NVML_CALL(nvmlDeviceGetClock(dev, NVML_CLOCK_MEM,
                    NVML_CLOCK_ID_CURRENT, &clock));
            fields.insert(std::pair(std::string("Clock Memory (MHz)"),
                std::to_string(clock)));

            /* Get PCIe throughput */
            uint32_t throughput = 0;
            NVML_CALL(nvmlDeviceGetPcieThroughput(dev,
                NVML_PCIE_UTIL_TX_BYTES, &throughput));
            fields.insert(std::pair(std::string("PCIe TX Throughput (MB/s)"),
                std::to_string(throughput * PCIE_THROUGHPUT)));
            NVML_CALL(nvmlDeviceGetPcieThroughput(dev,
                NVML_PCIE_UTIL_RX_BYTES, &throughput));
            fields.insert(std::pair(std::string("PCIe RX Throughput (MB/s)"),
                std::to_string(throughput * PCIE_THROUGHPUT)));

            /* Power */
            uint32_t power = 0;
            NVML_CALL(nvmlDeviceGetPowerUsage(dev, &power));
            fields.insert(std::pair(std::string("Power (W)"),
                std::to_string(power * WATTS)));
            uint32_t temperature = 0;
            NVML_CALL(nvmlDeviceGetTemperature(dev, NVML_TEMPERATURE_GPU,
                &temperature));
            fields.insert(std::pair(std::string("Temperature (C)"),
                std::to_string(temperature)));

            /* NVLink */
            nvmlFieldValue_t values2[2];
            int valuesCount{2};
            values2[0].fieldId = NVML_FI_DEV_NVLINK_SPEED_MBPS_COMMON;
            values2[1].fieldId = NVML_FI_DEV_NVLINK_LINK_COUNT;
            NVML_CALL(nvmlDeviceGetFieldValues(dev,
                valuesCount, values2));
            fields.insert(std::pair(std::string("NvLink Speed (GB/s)"),
                std::to_string(convertValue(values2[0]) * NVLINK_BW)));
            fields.insert(std::pair(std::string("NvLink Link Count"),
                std::to_string(convertValue(values2[1]))));

            nvmlFieldValue_t values6[6];
            values6[0].fieldId = NVML_FI_DEV_NVLINK_BANDWIDTH_C0_TOTAL;
            values6[1].fieldId = NVML_FI_DEV_NVLINK_BANDWIDTH_C1_TOTAL;
            values6[2].fieldId = NVML_FI_DEV_NVLINK_THROUGHPUT_DATA_TX;
            values6[3].fieldId = NVML_FI_DEV_NVLINK_THROUGHPUT_DATA_RX;
            values6[4].fieldId = NVML_FI_DEV_NVLINK_THROUGHPUT_RAW_TX;
            values6[5].fieldId = NVML_FI_DEV_NVLINK_THROUGHPUT_RAW_RX;
            values6[2].scopeId = UINT_MAX;
            values6[3].scopeId = UINT_MAX;
            values6[4].scopeId = UINT_MAX;
            values6[5].scopeId = UINT_MAX;
            valuesCount = 6;
            NVML_CALL(nvmlDeviceGetFieldValues(dev,
                valuesCount, values6));
            fields.insert(std::pair(std::string("NvLink Bandwidth C0 Total"),
                std::to_string(convertValue(values6[0]))));
            fields.insert(std::pair(std::string("NvLink Bandwidth C1 Total"),
                std::to_string(convertValue(values6[1]))));
            fields.insert(std::pair(std::string("NvLink Throughput Data TX"),
                std::to_string(convertValue(values6[2]))));
            fields.insert(std::pair(std::string("NvLink Throughput Data RX"),
                std::to_string(convertValue(values6[3]))));
            fields.insert(std::pair(std::string("NvLink Throughput Raw TX"),
                std::to_string(convertValue(values6[4]))));
            fields.insert(std::pair(std::string("NvLink Throughput Raw RX"),
                std::to_string(convertValue(values6[5]))));
            fields.insert(std::pair("step",std::to_string(step)));

            allfields.push_back(fields);
        }
        computeNode.updateGPU(allfields);
        return 0;
    }
}

