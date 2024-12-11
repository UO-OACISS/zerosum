#PATH=$PATH:$HOME/src/apex/install_gilgamesh_5.2.0/bin
export ROCM_COMPILER_VERSION=5.7.1
module unload darshan-runtime

builddir=`pwd`/build.${ROCM_COMPILER_VERSION}
instdir=`pwd`/install.${ROCM_COMPILER_VERSION}
#instdir=/sw/frontier/ums/ums002/zerosum/amd_${ROCM_COMPILER_VERSION}
#instdir=/lustre/orion/world-shared/phy122/khuck/zerosum.amd_${ROCM_COMPILER_VERSION}
rm -rf ${builddir}

cmake -B ${builddir} \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DCMAKE_INSTALL_PREFIX=${instdir} \
-DROCM_PATH=/opt/rocm-${ROCM_COMPILER_VERSION} \
-DZeroSum_WITH_HIP=TRUE \
-DZeroSum_WITH_HWLOC=TRUE \

cmake --build ${builddir} -j
cmake --install ${builddir}
#ctest --test-dir build

#-DZeroSum_WITH_PerfStubs=FALSE \
#-DCMAKE_CXX_COMPILER=`which CC` \
#-DCMAKE_C_COMPILER=`which cc` \