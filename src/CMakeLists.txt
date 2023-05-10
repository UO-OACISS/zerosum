CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/zerosum
    ${PROJECT_BINARY_DIR}/bin/zerosum @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/zerosum-mpi
    ${PROJECT_BINARY_DIR}/bin/zerosum-mpi @ONLY)

if (ZeroSum_STANDALONE)
    add_definitions(-DZEROSUM_STANDALONE)
endif ()
if (ZeroSum_WITH_PerfStubs)
    add_definitions(-DPERFSTUBS_USE_TIMERS)
    set(PERFSTUBS_LIB perfstubs)
endif ()
set(SOURCES zerosum_openmp.cpp zerosum.cpp zerosum_nogpu.cpp zerosum_pthreads.cpp utils.cpp)

add_library(zerosum SHARED ${SOURCES})
target_link_libraries (zerosum ${PERFSTUBS_LIB} OpenMP::OpenMP_CXX pthread stdc++)
add_dependencies (zerosum.tests zerosum)

add_library(zerosum-mpi SHARED ${SOURCES})
target_compile_definitions(zerosum-mpi PUBLIC -DUSE_MPI=1)
target_link_libraries (zerosum-mpi ${PERFSTUBS_LIB} MPI::MPI_CXX OpenMP::OpenMP_CXX pthread stdc++)
add_dependencies (zerosum.tests zerosum-mpi)

