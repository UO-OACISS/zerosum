CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/zerosum
    ${PROJECT_BINARY_DIR}/bin/zerosum @ONLY)

if (ZeroSum_STANDALONE)
    add_definitions(-DZEROSUM_STANDALONE)
endif ()
if (ZeroSum_WITH_PerfStubs)
    add_definitions(-DPERFSTUBS_USE_TIMERS)
    set(PERFSTUBS_LIB perfstubs)
endif ()
set(SOURCES zerosum_openmp.cpp zerosum.cpp zerosum_process.cpp zerosum_nogpu.cpp zerosum_pthreads.cpp)
add_library(zerosum SHARED ${SOURCES})
target_link_libraries (zerosum ${PERFSTUBS_LIB} MPI::MPI_CXX OpenMP::OpenMP_CXX pthread stdc++)
add_dependencies (zerosum.tests zerosum)
