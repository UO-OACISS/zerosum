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

add_executable(zs_mpi_p2p_merge mpi_p2p_merge.cpp)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/hwloc_util.py
    ${PROJECT_BINARY_DIR}/bin/zs-hwloc-sunburst.py @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/zs-stacks.py
    ${PROJECT_BINARY_DIR}/bin/zs-stacks.py @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/ctx.py
    ${PROJECT_BINARY_DIR}/bin/zs-ctx.py @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/heatmap.all_in_one.py
    ${PROJECT_BINARY_DIR}/bin/zs-heatmap-all-in-one.py @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/heatmap.py
    ${PROJECT_BINARY_DIR}/bin/zs-heatmap.py @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/hwt.py
    ${PROJECT_BINARY_DIR}/bin/zs-hwt.py @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/lwp.py
    ${PROJECT_BINARY_DIR}/bin/zs-lwp.py @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/sparklines.py
    ${PROJECT_BINARY_DIR}/bin/zs-sparklines.py @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/time-series.py
    ${PROJECT_BINARY_DIR}/bin/zs-time-series.py @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/trees.py
    ${PROJECT_BINARY_DIR}/bin/zs-trees.py @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/hwloc-template.html
    ${PROJECT_BINARY_DIR}/etc/zs-hwloc-template.html @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/hwloc-template-standalone.html
    ${PROJECT_BINARY_DIR}/etc/zs-hwloc-template-standalone.html @ONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/src/post-processing/d3.v3.js
    ${PROJECT_BINARY_DIR}/etc/d3.v3.js @ONLY)

INSTALL(FILES
    ${PROJECT_BINARY_DIR}/bin/zs-hwloc-sunburst.py
    ${PROJECT_BINARY_DIR}/bin/zs-stacks.py
    ${PROJECT_BINARY_DIR}/bin/zs-ctx.py
    ${PROJECT_BINARY_DIR}/bin/zs-heatmap-all-in-one.py
    ${PROJECT_BINARY_DIR}/bin/zs-heatmap.py
    ${PROJECT_BINARY_DIR}/bin/zs-hwt.py
    ${PROJECT_BINARY_DIR}/bin/zs-lwp.py
    ${PROJECT_BINARY_DIR}/bin/zs-sparklines.py
    ${PROJECT_BINARY_DIR}/bin/zs-time-series.py
    ${PROJECT_BINARY_DIR}/bin/zs-trees.py
    DESTINATION bin
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    WORLD_EXECUTE WORLD_READ)

INSTALL(FILES
    ${PROJECT_BINARY_DIR}/etc/zs-hwloc-template.html
    ${PROJECT_BINARY_DIR}/etc/zs-hwloc-template-standalone.html
    ${PROJECT_BINARY_DIR}/etc/d3.v3.js
    DESTINATION etc
    PERMISSIONS OWNER_WRITE OWNER_READ
    GROUP_READ
    WORLD_READ)
