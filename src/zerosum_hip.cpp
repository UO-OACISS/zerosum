/**********************************************************
"Hello World"-type program to test different srun layouts.

Written by Tom Papatheodore
**********************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <set>
#include <iomanip>
#include <string.h>
#include "hip/hip_runtime.h"
#include "zerosum.h"

// Macro for checking errors in GPU API calls
#define gpuErrorCheck(call)                                                                  \
do{                                                                                          \
    hipError_t gpuErr = call;                                                               \
    if(hipSuccess != gpuErr){                                                               \
        printf("GPU Error - %s:%d: '%s'\n", __FILE__, __LINE__, hipGetErrorString(gpuErr)); \
        exit(0);                                                                             \
    }                                                                                        \
}while(0)

namespace zerosum {

int ZeroSum::getgpu(void) {
    const char* gpu_id_list;
    std::vector<uint32_t> device_ids;

    // If HIP_VISIBLE_DEVICES is set, capture visible GPUs
    const char* gpu_visible_devices = getenv("ROCR_VISIBLE_DEVICES");
    if(gpu_visible_devices == NULL){
       	gpu_id_list = "N/A";
    }
    else{
       	gpu_id_list = gpu_visible_devices;
        device_ids = parseDiscreteValues(std::string(gpu_visible_devices));
    }

	// Find how many GPUs runtime says are available
	int num_devices = 0;
    gpuErrorCheck( hipGetDeviceCount(&num_devices) );

    std::vector<std::map<std::string,std::string>> fieldVector;
	if(num_devices > 0){
		char busid[64];
        std::string busid_list = "";
        std::string rt_gpu_id_list = "";

		// Loop over the GPUs available to each MPI rank
		for(int i=0; i<num_devices; i++){
            std::map<std::string,std::string> fields;
            fields.insert(std::pair(std::string("RT_GPU_ID"), std::to_string(i)));
            if (device_ids.size() > (unsigned)i) {
                fields.insert(std::pair(std::string("GPU_ID"), std::to_string(device_ids[i])));
            } else {
                fields.insert(std::pair(std::string("GPU_ID"), std::to_string(i)));
            }
			gpuErrorCheck( hipSetDevice(i) );

			// Get the PCIBusId for each GPU and use it to query for UUID
			gpuErrorCheck( hipDeviceGetPCIBusId(busid, 64, i) );
            fields.insert(std::pair(std::string("Bus ID"), std::string(busid)));

            // get lots of stuff
            hipDeviceProp_t prop;
            gpuErrorCheck(hipGetDeviceProperties(&prop, i));
            fields.insert(std::pair("Name", std::string(prop.name)));
            fields.insert(std::pair("Total Global Memory", std::to_string(prop.totalGlobalMem)));
            fields.insert(std::pair("Shared Memory per Block", std::to_string(prop.sharedMemPerBlock)));
            fields.insert(std::pair("Registers per Block", std::to_string(prop.regsPerBlock)));
            fields.insert(std::pair("Warp Size", std::to_string(prop.warpSize)));
            fields.insert(std::pair("Max Threads per Block", std::to_string(prop.maxThreadsPerBlock)));
            std::string tmp{"[" + std::to_string(prop.maxThreadsDim[0]) + ","
                                + std::to_string(prop.maxThreadsDim[1]) + ","
                                + std::to_string(prop.maxThreadsDim[2]) + "]"};
            fields.insert(std::pair("Max Grid Size", tmp));
            tmp = "[" + std::to_string(prop.maxGridSize[0]) + ","
                      + std::to_string(prop.maxGridSize[1]) + ","
                      + std::to_string(prop.maxGridSize[2]) + "]";
            fields.insert(std::pair("Max Threads Dim", tmp));
            fields.insert(std::pair("Clock Rate", std::to_string(prop.clockRate)));
            fields.insert(std::pair("Memory Clock Rate", std::to_string(prop.memoryClockRate)));
            fields.insert(std::pair("Memory Bus Width", std::to_string(prop.memoryBusWidth)));
            fields.insert(std::pair("Total Const Memory", std::to_string(prop.totalConstMem)));
            fields.insert(std::pair("Major Compute", std::to_string(prop.major)));
            fields.insert(std::pair("Minor Compute", std::to_string(prop.minor)));
            fields.insert(std::pair("Multi Processor Count", std::to_string(prop.multiProcessorCount)));
            fields.insert(std::pair("L2 Cache Size", std::to_string(prop.l2CacheSize)));
            fields.insert(std::pair("Max Threads per Multi Processor", std::to_string(prop.maxThreadsPerMultiProcessor)));
            fields.insert(std::pair("Compute Mode", std::to_string(prop.computeMode)));
            fields.insert(std::pair("Clock Instruction Rate", std::to_string(prop.clockInstructionRate)));
            //fields.insert(std::pair("Arch", std::to_string(prop.arch)));
            fields.insert(std::pair("Concurrent Kernels", std::string(prop.concurrentKernels > 0 ? "yes" : "no")));
            fields.insert(std::pair("PCI Bus ID", std::to_string(prop.pciBusID)));
            fields.insert(std::pair("PCI Device ID", std::to_string(prop.pciDeviceID)));
            fields.insert(std::pair("Max Shared Memory per Multi Processor", std::to_string(prop.maxSharedMemoryPerMultiProcessor)));
            fields.insert(std::pair("Is Multi GPU Board", std::string(prop.isMultiGpuBoard > 0 ? "yes" : "no")));
            fields.insert(std::pair("Can Map Host Memory", std::string(prop.canMapHostMemory > 0 ? "yes" : "no")));
            fields.insert(std::pair("AMD GCN Arch", std::to_string(prop.gcnArch)));

			// Concatenate per-MPIrank GPU info into strings for print
            if(i > 0) rt_gpu_id_list.append(",");
            rt_gpu_id_list.append(std::to_string(i));

            std::string temp_busid(busid);

            if(i > 0) busid_list.append(",");
            busid_list.append(temp_busid);
            char buffer[1025];
            snprintf(buffer, 1024,
                "MPI %03d - Node %s - RT_GPU_ID %s - GPU_ID %s - Bus_ID %s",
                ZeroSum::getInstance().process.rank,
                ZeroSum::getInstance().computeNode.name.c_str(),
                rt_gpu_id_list.c_str(),
                gpu_id_list,
                busid_list.c_str());
            logfile << buffer << std::endl;
            fieldVector.push_back(fields);
		}
	}
    ZeroSum::getInstance().computeNode.addGpu(fieldVector);
	return 0;
}

}
