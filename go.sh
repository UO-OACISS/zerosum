
rm -rf build

cmake -B build \
-DCMAKE_CXX_COMPILER=`which amdclang++` \
-DCMAKE_C_COMPILER=`which amdclang` \
-DZeroSum_STANDALONE=TRUE \

cmake --build build -j8