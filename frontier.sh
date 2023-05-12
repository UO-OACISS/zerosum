PATH=$PATH:$HOME/src/apex/install_gilgamesh_5.2.0/bin

rm -rf build

cmake -B build \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DCMAKE_INSTALL_PREFIX=`pwd`/install \
-DZeroSum_STANDALONE=TRUE \

cmake --build build -j
cmake --install build
#ctest --test-dir build

#-DZeroSum_WITH_PerfStubs=FALSE \
#-DCMAKE_CXX_COMPILER=`which CC` \
#-DCMAKE_C_COMPILER=`which cc` \