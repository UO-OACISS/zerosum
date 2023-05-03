/**********************************************************
"Hello World"-type program to test different srun layouts.

Written by Tom Papatheodore
**********************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE             /* See feature_test_macros(7) */
#endif
#define _XOPEN_SOURCE 700
#include <sched.h>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <set>
#include <iomanip>
#include <iomanip>
#include <string.h>
#include <mpi.h>
#include <omp.h>
#include <hip/hip_runtime.h>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/syscall.h>
#define gettid() syscall(SYS_gettid)

// Macro for checking errors in GPU API calls
#define gpuErrorCheck(call)                                                                  \
do{                                                                                          \
    hipError_t gpuErr = call;                                                               \
    if(hipSuccess != gpuErr){                                                               \
        printf("GPU Error - %s:%d: '%s'\n", __FILE__, __LINE__, hipGetErrorString(gpuErr)); \
        exit(0);                                                                             \
    }                                                                                        \
}while(0)

int getgpu(const int rank, const int section, const char * name) {
    const char* gpu_id_list;

    // If HIP_VISIBLE_DEVICES is set, capture visible GPUs
    const char* gpu_visible_devices = getenv("ROCR_VISIBLE_DEVICES");
    if(gpu_visible_devices == NULL){
       	gpu_id_list = "N/A";
    }
    else{
       	gpu_id_list = gpu_visible_devices;
    }

	// Find how many GPUs runtime says are available
	int num_devices = 0;
    gpuErrorCheck( hipGetDeviceCount(&num_devices) );

	if(num_devices > 0){
		char busid[64];
        std::string busid_list = "";
        std::string rt_gpu_id_list = "";

		// Loop over the GPUs available to each MPI rank
		for(int i=0; i<num_devices; i++){

			gpuErrorCheck( hipSetDevice(i) );

			// Get the PCIBusId for each GPU and use it to query for UUID
			gpuErrorCheck( hipDeviceGetPCIBusId(busid, 64, i) );

			// Concatenate per-MPIrank GPU info into strings for print
            if(i > 0) rt_gpu_id_list.append(",");
            rt_gpu_id_list.append(std::to_string(i));

            std::string temp_busid(busid);

            if(i > 0) busid_list.append(",");
//            busid_list.append(temp_busid.substr(8,2));
            busid_list.append(temp_busid);

            printf("MPI %03d - SEC %d - Node %s - RT_GPU_ID %s - GPU_ID %s - Bus_ID %s\n",
                rank, section, name, rt_gpu_id_list.c_str(), gpu_id_list, busid_list.c_str());
		}
	}
	return 0;
}
