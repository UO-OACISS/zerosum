PATH=$PATH:$HOME/src/apex/install_gilgamesh_5.2.0/bin

rm -rf build

cmake -B build \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DCMAKE_CXX_COMPILER=`which amdclang++` \
-DCMAKE_C_COMPILER=`which amdclang` \
-DCMAKE_INSTALL_PREFIX=`pwd`/install \
-DCMAKE_C_FLAGS=-gdwarf-4 \
-DCMAKE_CXX_FLAGS=-gdwarf-4 \
-DZeroSum_STANDALONE=TRUE \

cmake --build build -j
cmake --install build
ctest --test-dir build

#-DZeroSum_WITH_PerfStubs=FALSE \