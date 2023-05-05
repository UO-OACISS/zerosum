add_executable(lu-decomp main.cpp)
target_link_libraries (lu-decomp MPI::MPI_CXX OpenMP::OpenMP_CXX pthread stdc++)
add_dependencies (zerosum.tests lu-decomp)

add_test (NAME test_lu-decomp COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4
    --bind-to core --cpus-per-proc 4
    ${CMAKE_BINARY_DIR}/bin/zerosum ${CMAKE_BINARY_DIR}/bin/lu-decomp)

set_property (TEST test_lu-decomp PROPERTY
    ENVIRONMENT "OMP_NUM_THREADS=4;OMP_PROC_BIND=spread;OMP_PLACES=sockets")

add_test (NAME test_lu-decomp_apex COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4
    --bind-to core --cpus-per-proc 4
    apex_exec --apex:mpi --apex:ompt --apex:pthread --apex:ompt_details --apex:tasktree --apex:gtrace
    --apex:preload ${CMAKE_BINARY_DIR}/lib/libzerosum.so
    ${CMAKE_BINARY_DIR}/bin/lu-decomp)

set_property (TEST test_lu-decomp_apex PROPERTY
    ENVIRONMENT "OMP_NUM_THREADS=4;OMP_PROC_BIND=spread;OMP_PLACES=sockets")

