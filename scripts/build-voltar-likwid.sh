builddir=`pwd`/build.voltar
instdir=`pwd`/install.voltar
rm -rf ${builddir} ${instdir}

export PKG_CONFIG_PATH=${PKG_CONFIG_PATH}:/home/users/khuck/src/hwloc-2.4.0/install_voltar_gcc/lib/pkgconfig
cmake -B ${builddir} \
-DCMAKE_CXX_COMPILER=`which g++` \
-DCMAKE_C_COMPILER=`which gcc` \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DCMAKE_INSTALL_PREFIX=${instdir} \
-DCMAKE_MODULE_PATH=/usr/local/packages/likwid/gcc8.5-cuda11.7/share/likwid \
-DCMAKE_PREFIX_PATH=/usr/local/packages/likwid/gcc8.5-cuda11.7/share/likwid \
-DZeroSum_WITH_CUDA=TRUE \
-DZeroSum_WITH_MPI=FALSE \
-DZeroSum_WITH_LIKWID=TRUE \

#-DZeroSum_WITH_HWLOC=TRUE \
#-DZeroSum_WITH_LM_SENSORS=TRUE \
#-DLM_SENSORS_ROOT=${HOME}/src/lm-sensors/install \

#-DZeroSum_USE_STATIC_GLOBAL_CONSTRUCTOR=FALSE

cmake --build ${builddir} --parallel
cmake --install ${builddir}
ctest --test-dir ${builddir}

#-DZeroSum_WITH_PerfStubs=FALSE \
