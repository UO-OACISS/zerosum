add_executable(lu-decomp main.cpp)
target_link_libraries (lu-decomp MPI::MPI_CXX OpenMP::OpenMP_CXX pthread stdc++)
add_dependencies (zerosum.tests lu-decomp)

add_test (NAME test_lu-decomp COMMAND ${MPIEXEC} ${MPIEXEC_NUMPROC_FLAG} 4
    --bind-to core --cpus-per-proc 4
    ${CMAKE_BINARY_DIR}/bin/zerosum ${CMAKE_BINARY_DIR}/bin/lu-decomp)

set_property (TEST test_lu-decomp PROPERTY ENVIRONMENT OMP_NUM_THREADS=4)
set_property (TEST test_lu-decomp PROPERTY ENVIRONMENT OMP_PROC_BIND=spread)
set_property (TEST test_lu-decomp PROPERTY ENVIRONMENT OMP_PLACES=sockets)
