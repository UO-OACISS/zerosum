#
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
#

math(EXPR ZeroSum_LAST_CORE ${ZeroSum_NPROC}-1)

add_executable(deadlock deadlock.cpp)
target_link_libraries (deadlock pthread)
add_dependencies (deadlock zerosum)

if (ZeroSum_STANDALONE)
    add_executable(crasher crasher.c)
    target_link_libraries (crasher pthread)
    add_dependencies (crasher zerosum)
    add_dependencies (zerosum.tests crasher)

#    add_test (NAME test_crasher COMMAND taskset --cpu-list 0-${ZeroSum_LAST_CORE}
#        ${CMAKE_BINARY_DIR}/bin/crasher)
#    set_tests_properties(test_crasher PROPERTIES WILL_FAIL TRUE)
endif()

add_executable(lu-decomp main.cpp)
target_link_libraries (lu-decomp pthread)
if (ZeroSum_WITH_OPENMP)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVHPC")
        # Use the tool-enabled OpenMP runtime
        target_compile_options(lu-decomp PRIVATE "-mp=ompt")
        target_link_options(lu-decomp PRIVATE "-mp=ompt")
    else()
        target_link_libraries (lu-decomp OpenMP::OpenMP_CXX)
    endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVHPC")
endif (ZeroSum_WITH_OPENMP)
add_dependencies (lu-decomp zerosum)
add_dependencies (zerosum.tests lu-decomp)

add_test (NAME test_lu-decomp-serial COMMAND taskset --cpu-list 0-${ZeroSum_LAST_CORE}
    ${CMAKE_BINARY_DIR}/bin/zerosum ${CMAKE_BINARY_DIR}/bin/lu-decomp)

if (ZeroSum_WITH_OPENMP)
    set_property (TEST test_lu-decomp-serial PROPERTY
        ENVIRONMENT "OMP_NUM_THREADS=4;OMP_PROC_BIND=spread;OMP_PLACES=cores")
endif (ZeroSum_WITH_OPENMP)

# MPI Examples

if (ZeroSum_WITH_MPI)
    add_executable(deadlock-mpi deadlock.cpp)
    target_link_libraries (deadlock-mpi MPI::MPI_CXX pthread)
    target_compile_definitions(deadlock-mpi PUBLIC USE_MPI)
    add_dependencies (deadlock-mpi zerosum-mpi)

    add_executable(lu-decomp-mpi main.cpp)
    target_link_libraries (lu-decomp-mpi MPI::MPI_CXX pthread)
    if (ZeroSum_WITH_OPENMP)
        if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVHPC")
            # Use the tool-enabled OpenMP runtime
            target_compile_options(lu-decomp-mpi PRIVATE "-mp=ompt")
            target_link_options(lu-decomp-mpi PRIVATE "-mp=ompt")
        else()
            target_link_libraries (lu-decomp-mpi OpenMP::OpenMP_CXX)
        endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVHPC")
    endif (ZeroSum_WITH_OPENMP)
    target_compile_definitions(lu-decomp-mpi PUBLIC USE_MPI)
    add_dependencies (lu-decomp-mpi zerosum-mpi)
    add_dependencies (zerosum.tests lu-decomp-mpi)

    set(MPIEXEC_BIND_OPTIONS "")
    if(${MPIEXEC} MATCHES ".*srun")
        set(MPIEXEC_BIND_OPTIONS -c 4)
    endif()
    if(${MPIEXEC} MATCHES ".*mpiexec")
        set(MPIEXEC_BIND_OPTIONS --bind-to core --cpus-per-proc 4)
    endif()

    add_test (NAME test_lu-decomp-mpi COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4
        ${MPIEXEC_BIND_OPTIONS}
        ${CMAKE_BINARY_DIR}/bin/zerosum-mpi ${CMAKE_BINARY_DIR}/bin/lu-decomp-mpi)

    if (ZeroSum_WITH_OPENMP)
        set_property (TEST test_lu-decomp-mpi PROPERTY
            ENVIRONMENT "OMP_NUM_THREADS=4;OMP_PROC_BIND=spread;OMP_PLACES=cores")
    endif (ZeroSum_WITH_OPENMP)

    # MPI Example executed with APEX

    add_test (NAME test_lu-decomp_apex COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4
        ${MPIEXEC_BIND_OPTIONS} apex_exec
        --apex:mpi --apex:ompt --apex:pthread --apex:ompt_details
        --apex:tasktree --apex:gtrace --apex:screen_details
        --apex:preload ${CMAKE_BINARY_DIR}/lib/libzerosum-mpi.so
        ${CMAKE_BINARY_DIR}/bin/lu-decomp-mpi)

    if (ZeroSum_WITH_OPENMP)
        set_property (TEST test_lu-decomp_apex PROPERTY
            ENVIRONMENT "OMP_NUM_THREADS=4;OMP_PROC_BIND=close;OMP_PLACES=cores")
    endif (ZeroSum_WITH_OPENMP)

    add_executable(random_walk random_walk.cpp)
    target_link_libraries (random_walk MPI::MPI_CXX pthread)
    if (ZeroSum_WITH_OPENMP)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVHPC")
        # Use the tool-enabled OpenMP runtime
        target_compile_options(random_walk PRIVATE "-mp=ompt")
        target_link_options(random_walk PRIVATE "-mp=ompt")
    else()
        target_link_libraries (random_walk OpenMP::OpenMP_CXX)
    endif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "NVHPC")
    endif (ZeroSum_WITH_OPENMP)
    target_compile_definitions(random_walk PUBLIC USE_MPI)
    add_dependencies (random_walk zerosum-mpi)
    add_dependencies (zerosum.tests random_walk)

    add_test (NAME test_random_walk COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4
        ${MPIEXEC_BIND_OPTIONS}
        ${CMAKE_BINARY_DIR}/bin/zerosum-mpi ${CMAKE_BINARY_DIR}/bin/random_walk 10000 1000000 1000)
endif (ZeroSum_WITH_MPI)

if (ZeroSum_WITH_SYCL)
add_executable(sycl_query sycl_query.cpp)
add_dependencies (sycl_query zerosum)
if (IntelSYCL_FOUND)
target_link_libraries (sycl_query pthread IntelSYCL::SYCL_CXX ze_loader)
else (IntelSYCL_FOUND)
include_directories(${SYCL_ROOT}/include)
include_directories(${SYCL_ROOT}/include/sycl)
target_link_libraries (sycl_query pthread sycl ze_loader)
endif (IntelSYCL_FOUND)
endif (ZeroSum_WITH_SYCL)
