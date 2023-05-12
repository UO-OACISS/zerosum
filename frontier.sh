PATH=$PATH:$HOME/src/apex/install_gilgamesh_5.2.0/bin

rm -rf build

cmake -B build \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DZeroSum_STANDALONE=TRUE \

cmake --build build -j
#ctest --test-dir build

#-DZeroSum_WITH_PerfStubs=FALSE \
#-DCMAKE_CXX_COMPILER=`which CC` \
#-DCMAKE_C_COMPILER=`which cc` \