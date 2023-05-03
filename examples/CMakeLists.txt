add_executable(lu-decomp main.cpp)
target_link_libraries (lu-decomp MPI::MPI_CXX OpenMP::OpenMP_CXX pthread stdc++)
add_dependencies (zerosum.tests lu-decomp)
add_test (NAME test_lu-decomp COMMAND ${CMAKE_BINARY_DIR}/bin/lu-decomp)
