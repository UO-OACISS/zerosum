set -e 

#PATH=$PATH:$HOME/src/apex/install/bin

rm -rf build

cmake -B build \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DCMAKE_CXX_COMPILER=`which clang++` \
-DCMAKE_C_COMPILER=`which clang` \
-DCMAKE_INSTALL_PREFIX=`pwd`/install \
-DSYCL_ROOT=/soft/compilers/oneapi/upstream/install-12.2.91_04262024/install \
-DZeroSum_WITH_SYCL=TRUE \
-DZeroSum_STANDALONE=TRUE \
-DZeroSum_WITH_HWLOC=TRUE \

cmake --build build -j
cmake --install build
#ctest --test-dir build

#-DZeroSum_WITH_PerfStubs=FALSE \


