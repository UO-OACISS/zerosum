#
# MIT License
#
# Copyright (c) 2023-2025 University of Oregon, Kevin Huck
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

# - Try to find LibOMPT
# Once done this will define
#  OMPT_FOUND - System has OMPT
#  OMPT_INCLUDE_DIRS - The OMPT include directories
#  OMPT_LIBRARIES - The libraries needed to use OMPT
#  OMPT_DEFINITIONS - Compiler switches required for using OMPT

# First, check if the compiler has built-in support for OpenMP 5.0:
INCLUDE(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG(-fopenmp HAVE_OPENMP)
try_compile(APEX_HAVE_OMPT_NATIVE ${CMAKE_CURRENT_BINARY_DIR}/ompt_5.0_test
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/tests/ompt_5.0_test ompt_5.0_test ompt_5.0_test
    CMAKE_FLAGS -DCMAKE_CXX_FLAGS:STRING="${OpenMP_CXX_FLAGS}"
    -DCMAKE_CMAKE_EXE_LINKER_FLAGS:STRING="${OpenMP_CXX_FLAGS}")

if(${APEX_HAVE_OMPT_NATIVE})
    set(OMPT_FOUND TRUE)
    mark_as_advanced(OMPT_INCLUDE_DIR OMPT_LIBRARY)
    add_custom_target(project_ompt)
    message("Detected compiler has native OpenMP 5.0 OMPT support.")

    # Now check for 5.1 support
    try_compile(APEX_HAVE_OMPT_5.1_NATIVE ${CMAKE_CURRENT_BINARY_DIR}/ompt_5.1_test
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/tests/ompt_5.1_test ompt_5.1_test ompt_5.1_test
        CMAKE_FLAGS -DCMAKE_CXX_FLAGS:STRING="${OpenMP_CXX_FLAGS}"
        -DCMAKE_CMAKE_EXE_LINKER_FLAGS:STRING="${OpenMP_CXX_FLAGS}")

    if(${APEX_HAVE_OMPT_5.1_NATIVE})
        message("Detected compiler has native OpenMP 5.1 OMPT support.")
        add_definitions(-DAPEX_HAVE_OMPT_5_1)
    else()
        message("Detected compiler does not have native OpenMP 5.1 OMPT support.")
    endif()
endif(${APEX_HAVE_OMPT_NATIVE})

