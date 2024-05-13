# assuming the cudatoolkit module is loaded, CUDATOOLKIT_HOME will be set
# and CMake will be able to find what it needs.

rm -rf build

cmake -B build \
-DCMAKE_BUILD_TYPE=RelWithDebInfo \
-DCMAKE_INSTALL_PREFIX=$SCRATCH/zerosum \
-DZeroSum_WITH_CUDA=TRUE \

cmake --build build -j
cmake --install build
ctest --test-dir build --rerun-failed --output-on-failure

