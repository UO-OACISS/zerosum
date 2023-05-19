CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/zerosum
    ${PROJECT_BINARY_DIR}/bin/zerosum @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/zerosum-mpi
    ${PROJECT_BINARY_DIR}/bin/zerosum-mpi @ONLY)

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

set(SOURCES
    zerosum.cpp
    zerosum_openmp.cpp
    zerosum_process.cpp
    zerosum_pthreads.cpp
    utils.cpp
    ${GPU_SOURCE}
    ${ERROR_HANDLING})

add_library(zerosum SHARED ${SOURCES})
target_link_libraries (zerosum ${PERFSTUBS_LIB} ${GPU_LIB} OpenMP::OpenMP_CXX pthread stdc++)
add_dependencies (zerosum.tests zerosum)

add_library(zerosum-mpi SHARED ${SOURCES})
target_compile_definitions(zerosum-mpi PUBLIC -DUSE_MPI=1)
target_link_libraries (zerosum-mpi ${PERFSTUBS_LIB} ${GPU_LIB} MPI::MPI_CXX OpenMP::OpenMP_CXX pthread stdc++)
add_dependencies (zerosum.tests zerosum-mpi)

