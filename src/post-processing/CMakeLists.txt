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
