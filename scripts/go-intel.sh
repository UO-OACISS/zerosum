set -e

PATH=$PATH:$HOME/src/apex/install/bin
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$HOME/src/hwloc-2.4.0/install_omnia/lib/pkgconfig

rm -rf build

cmake -B build \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DCMAKE_CXX_COMPILER=`which icpx` \
-DCMAKE_C_COMPILER=`which icx` \
-DCMAKE_INSTALL_PREFIX=`pwd`/install \
-DZeroSum_WITH_SYCL=TRUE \
-DZeroSum_STANDALONE=TRUE \
-DZeroSum_WITH_HWLOC=TRUE \
-DHWLOC_ROOT=${HOME}/src/hwloc-2.4.0/install_omnia

cmake --build build -j
cmake --install build
#ctest --test-dir build

#-DZeroSum_WITH_PerfStubs=FALSE \


