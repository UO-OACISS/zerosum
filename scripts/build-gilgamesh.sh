builddir=`pwd`/build.gilgamesh
instdir=`pwd`/install.gilgamesh
rm -rf ${builddir}

export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}:/usr/local/packages/hwloc/2.4.0/lib/pkgconfig
cmake -B ${builddir} \
-DCMAKE_CXX_COMPILER=`which amdclang++` \
-DCMAKE_C_COMPILER=`which amdclang` \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_INSTALL_PREFIX=${instdir} \
-DZeroSum_WITH_CUDA=TRUE \
-DZeroSum_WITH_HWLOC=TRUE \
-DZeroSum_USE_STATIC_GLOBAL_CONSTRUCTOR=FALSE

cmake --build ${builddir} -j
cmake --install ${builddir}
ctest --test-dir ${builddir}

#-DZeroSum_WITH_PerfStubs=FALSE \
