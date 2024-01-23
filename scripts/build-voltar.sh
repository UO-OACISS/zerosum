builddir=`pwd`/build.voltar
instdir=`pwd`/install.voltar
rm -rf ${builddir}

export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}:/home/users/khuck/src/hwloc-2.4.0/install_voltar_gcc/lib/pkgconfig
cmake -B ${builddir} \
-DCMAKE_CXX_COMPILER=`which g++` \
-DCMAKE_C_COMPILER=`which gcc` \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_INSTALL_PREFIX=${instdir} \
-DZeroSum_WITH_CUDA=TRUE \
-DZeroSum_WITH_HWLOC=TRUE \
-DZeroSum_USE_STATIC_GLOBAL_CONSTRUCTOR=FALSE

cmake --build ${builddir} -j
cmake --install ${builddir}
ctest --test-dir ${builddir}

#-DZeroSum_WITH_PerfStubs=FALSE \
