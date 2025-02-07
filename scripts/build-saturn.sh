builddir=`pwd`/build.saturn
instdir=`pwd`/install.saturn
rm -rf ${builddir} ${instdir}

#export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}:/home/users/khuck/src/hwloc/2.4.0-install_saturn/lib/pkgconfig
export PKG_CONFIG_PATH=/packages/hwloc/2.11.2/gcc9.4.0/lib/pkgconfig:${PKG_CONFIG_PATH}
cmake -B ${builddir} \
-DCMAKE_CXX_COMPILER=`which g++` \
-DCMAKE_C_COMPILER=`which gcc` \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_INSTALL_PREFIX=${instdir} \
-DZeroSum_WITH_CUDA=TRUE \
-DZeroSum_WITH_HWLOC=TRUE \
-DZeroSum_WITH_MPI=TRUE

cmake --build ${builddir} -j
cmake --install ${builddir}
ctest --test-dir ${builddir}

#-DZeroSum_WITH_PerfStubs=FALSE \
