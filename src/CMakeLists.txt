# MIT License
#
# Copyright (c) 2023 University of Oregon, Kevin Huck
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set(ZS_LIBNAME "libzerosum")
set(ZS_USE_MPI "no")
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/zerosum
    ${PROJECT_BINARY_DIR}/bin/zerosum @ONLY)

if (ZeroSum_WITH_MPI)
    set(ZS_LIBNAME "libzerosum-mpi")
    set(ZS_USE_MPI "yes")
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/zerosum
        ${PROJECT_BINARY_DIR}/bin/zerosum-mpi @ONLY)
    unset(ZS_LIBNAME)
    unset(ZS_USE_MPI)
endif (ZeroSum_WITH_MPI)

if (ZeroSum_STANDALONE)
    add_definitions(-DZEROSUM_STANDALONE)
    set(ERROR_HANDLING error_handling.cpp)
endif ()

if (ZeroSum_WITH_PerfStubs)
    add_definitions(-DPERFSTUBS_USE_TIMERS)
    set(PERFSTUBS_LIB perfstubs)
endif ()

set(GPU_SOURCE zerosum_nogpu.cpp)
if (ZeroSum_WITH_HIP)
    add_definitions(-DUSE_HIP)
    set(GPU_LIB rocm_smi64 hip::host)
    set(GPU_SOURCE zerosum_hip.cpp rocmsmi.cpp)
endif ()

if (ZeroSum_WITH_CUDA)
    add_definitions(-DUSE_CUDA)
    set(GPU_LIB CUDA::cudart CUDA::nvml)
    set(GPU_SOURCE zerosum_cuda.cpp nvml.cpp)
endif ()

if (ZeroSum_WITH_SYCL)
    add_definitions(-DUSE_SYCL)
    if(IntelSYCL_FOUND)
    	set(GPU_LIB IntelSYCL::SYCL_CXX ze_loader)
    else()
        include_directories(${SYCL_ROOT}/include)
        include_directories(${SYCL_ROOT}/include/sycl)
	set (GPU_LIB sycl ze_loader)
    endif()
    set(GPU_SOURCE zerosum_sycl.cpp sycl.cpp)
endif ()

if (ZeroSum_WITH_HWLOC)
    set(HWLOC_SOURCE hwloc.cpp)
    set(HWLOC_LIB PkgConfig::HWLOC)
endif (ZeroSum_WITH_HWLOC)

if (ZeroSum_WITH_LM_SENSORS)
    set(LM_SENSORS_SOURCE lm_sensor_data.cpp)
    include_directories(${LM_SENSORS_INCLUDE_DIRS})
endif (ZeroSum_WITH_LM_SENSORS)

if (ZeroSum_WITH_OPENMP)
    set(OPENMP_SOURCE zerosum_openmp.cpp)
endif (ZeroSum_WITH_OPENMP)

set(SOURCES
    zerosum.cpp
    ${OPENMP_SOURCE}
    zerosum_process.cpp
    zerosum_pthreads.cpp
    utils.cpp
    cray_pm_counters.cpp
    ${GPU_SOURCE}
    ${HWLOC_SOURCE}
    ${LM_SENSORS_SOURCE}
    ${ERROR_HANDLING})

# Single process library

add_library(zerosum SHARED ${SOURCES})
target_link_libraries (zerosum PUBLIC ${LM_SENSORS_LIBRARIES} ${PERFSTUBS_LIB} ${GPU_LIB} ${HWLOC_LIB} ${LM_SENSORS_LIB} pthread)
if (ZeroSum_WITH_OPENMP)
    target_compile_definitions(zerosum PUBLIC -DZEROSUM_USE_OPENMP=1)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVHPC")
        # Use the tool-enabled OpenMP runtime
        target_compile_options(zerosum PRIVATE "-mp=ompt")
        target_link_options(zerosum PRIVATE "-mp=ompt")
    else()
        target_link_libraries (zerosum PUBLIC OpenMP::OpenMP_CXX)
    endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVHPC")
    if(OMPT_FOUND)
        target_compile_definitions(zerosum PUBLIC -DZEROSUM_USE_OMPT=1)
    endif(OMPT_FOUND)
endif (ZeroSum_WITH_OPENMP)
add_dependencies (zerosum.tests zerosum)
if (ZeroSum_WITH_LM_SENSORS AND NOT LM_SENSORS_FOUND)
    add_dependencies (zerosum lmsensors)
endif (ZeroSum_WITH_LM_SENSORS AND NOT LM_SENSORS_FOUND)

# MPI library

if (ZeroSum_WITH_MPI)
    add_library(zerosum-mpi SHARED ${SOURCES} zerosum_mpi.cpp)
    target_compile_definitions(zerosum-mpi PUBLIC -DZEROSUM_USE_MPI=1)
    target_link_libraries (zerosum-mpi PUBLIC ${LM_SENSORS_LIBRARIES} ${PERFSTUBS_LIB} ${GPU_LIB} ${HWLOC_LIB} MPI::MPI_CXX pthread)
    if (ZeroSum_WITH_OPENMP)
        target_compile_definitions(zerosum-mpi PUBLIC -DZEROSUM_USE_OPENMP=1)
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVHPC")
            # Use the tool-enabled OpenMP runtime
            target_compile_options(zerosum-mpi PRIVATE "-mp=ompt")
            target_link_options(zerosum-mpi PRIVATE "-mp=ompt")
        else()
            target_link_libraries (zerosum-mpi PUBLIC OpenMP::OpenMP_CXX)
        endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVHPC")
        if(OMPT_FOUND)
            target_compile_definitions(zerosum-mpi PUBLIC -DZEROSUM_USE_OMPT=1)
        endif(OMPT_FOUND)
    endif (ZeroSum_WITH_OPENMP)
    add_dependencies (zerosum.tests zerosum-mpi)
    if (ZeroSum_WITH_LM_SENSORS AND NOT LM_SENSORS_FOUND)
        add_dependencies (zerosum lmsensors)
    endif (ZeroSum_WITH_LM_SENSORS AND NOT LM_SENSORS_FOUND)
endif (ZeroSum_WITH_MPI)

