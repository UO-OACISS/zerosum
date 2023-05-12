# Example without MPI

if (ZeroSum_STANDALONE)
    add_executable(crasher crasher.c)
    target_link_libraries (crasher zerosum pthread stdc++)
    add_dependencies (zerosum.tests crasher)

    add_test (NAME test_crasher COMMAND taskset --cpu-list 0-7
        ${CMAKE_BINARY_DIR}/bin/crasher)
    set_tests_properties(test_crasher PROPERTIES WILL_FAIL TRUE)
endif()

add_executable(lu-decomp main.cpp)
target_link_libraries (lu-decomp zerosum OpenMP::OpenMP_CXX pthread stdc++)
add_dependencies (zerosum.tests lu-decomp)

add_test (NAME test_lu-decomp COMMAND taskset --cpu-list 0-7
    ${CMAKE_BINARY_DIR}/bin/lu-decomp)

set_property (TEST test_lu-decomp PROPERTY
    ENVIRONMENT "OMP_NUM_THREADS=4;OMP_PROC_BIND=spread;OMP_PLACES=sockets")

# MPI Example

add_executable(lu-decomp-mpi main.cpp)
target_link_libraries (lu-decomp-mpi MPI::MPI_CXX OpenMP::OpenMP_CXX pthread stdc++)
target_compile_definitions(lu-decomp-mpi PUBLIC USE_MPI)
add_dependencies (zerosum.tests lu-decomp-mpi)

add_test (NAME test_lu-decomp-mpi COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4
    --bind-to core --cpus-per-proc 4
    ${CMAKE_BINARY_DIR}/bin/zerosum-mpi ${CMAKE_BINARY_DIR}/bin/lu-decomp-mpi)

set_property (TEST test_lu-decomp-mpi PROPERTY
    ENVIRONMENT "OMP_NUM_THREADS=4;OMP_PROC_BIND=spread;OMP_PLACES=sockets")

# MPI Example executed with APEX

add_test (NAME test_lu-decomp_apex COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4
    --bind-to core --cpus-per-proc 4 apex_exec
    --apex:mpi --apex:ompt --apex:pthread --apex:ompt_details
    --apex:tasktree --apex:gtrace --apex:screen_details
    --apex:preload ${CMAKE_BINARY_DIR}/lib/libzerosum-mpi.so
    ${CMAKE_BINARY_DIR}/bin/lu-decomp-mpi)

set_property (TEST test_lu-decomp_apex PROPERTY
    ENVIRONMENT "OMP_NUM_THREADS=4;OMP_PROC_BIND=close;OMP_PLACES=cores")

