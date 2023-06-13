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


cmake_minimum_required(VERSION 3.20.1) # Minimum required for CUDA

# set the project name and version
project(ZeroSum VERSION 1.0 LANGUAGES C CXX)

option (ZeroSum_WITH_PerfStubs "Enable PerfStubs support" TRUE)
option (ZeroSum_WITH_HIP "Enable HIP support" FALSE)
option (ZeroSum_WITH_CUDA "Enable CUDA support" FALSE)
option (ZeroSum_STANDALONE "Enable standalone support" FALSE)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
add_compile_options(-Wall -Wextra -pedantic)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

if (ZeroSum_WITH_HIP)
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH} ${ROCM_ROOT}/lib/cmake")
    find_package(hip REQUIRED)
    find_package(rocm_smi REQUIRED)
    add_definitions(-DUSE_HIP)
endif (ZeroSum_WITH_HIP)

if (ZeroSum_WITH_CUDA)
    find_package(CUDAToolkit REQUIRED VERBOSE COMPONENTS cuPTI nvidia-ML )
    add_definitions(-DUSE_CUDA)
endif (ZeroSum_WITH_CUDA)

if (ZeroSum_WITH_SYCL)
    find_package(IntelSYCL REQUIRED VERBOSE)
    add_definitions(-DUSE_SYCL)
endif (ZeroSum_WITH_SYCL)

find_package(MPI REQUIRED)
find_package(OpenMP REQUIRED)
include(cmake/AddGitSubmodule.cmake)

enable_testing()

if (ZeroSum_WITH_PerfStubs)
    add_git_submodule(perfstubs)
endif (ZeroSum_WITH_PerfStubs)

add_custom_target(zerosum.tests)
add_custom_command(TARGET zerosum.tests POST_BUILD COMMAND ctest -R test --output-on-failure --timeout 100)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# RPATH settings

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")

if (ZeroSum_USE_STATIC_GLOBAL_CONSTRUCTOR OR APPLE)
    add_definitions(-DZEROSUM_USE_STATIC_GLOBAL_CONSTRUCTOR)
endif()

add_subdirectory(src)
add_subdirectory(examples)

function(dump_cmake_variables)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif()
        endif()
        message(STATUS "${_variableName} = ${${_variableName}}")
    endforeach()
endfunction()

INSTALL(TARGETS zerosum zerosum-mpi
        EXPORT ZeroSumTargets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION include)

install(EXPORT ZeroSumTargets
        FILE ZeroSumTargets.cmake
        NAMESPACE ZeroSum::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ZeroSum)

install(FILES
    ${PROJECT_BINARY_DIR}/bin/zerosum
    ${PROJECT_BINARY_DIR}/bin/zerosum-mpi
    DESTINATION bin
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ
    )

message(STATUS "----------------------------------------------------------------------")
message(STATUS "ZeroSum Variable Report:")
message(STATUS "----------------------------------------------------------------------")
dump_cmake_variables("^ZeroSum")
MESSAGE(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
MESSAGE(STATUS "Libraries: " ${LIBS})
MESSAGE(STATUS "Compiler cxx debug flags:" ${CMAKE_CXX_FLAGS_DEBUG})
MESSAGE(STATUS "Compiler cxx release flags:" ${CMAKE_CXX_FLAGS_RELEASE})
MESSAGE(STATUS "Compiler cxx min size flags:" ${CMAKE_CXX_FLAGS_MINSIZEREL})
MESSAGE(STATUS "Compiler cxx flags:" ${CMAKE_CXX_FLAGS})
MESSAGE(STATUS "Install Prefix:" ${CMAKE_INSTALL_PREFIX})
message(STATUS "----------------------------------------------------------------------")
