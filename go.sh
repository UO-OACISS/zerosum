
rm -rf build

cmake -B build \
-DCMAKE_CXX_COMPILER=`which amdclang++` \
-DCMAKE_C_COMPILER=`which amdclang` \
-DZeroSum_STANDALONE=TRUE \
-DZeroSum_WITH_PerfStubs=FALSE \

cmake --build build -v
ctest -V --test-dir build