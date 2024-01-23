
module load nvhpc/22.11

PATH=$PATH:$HOME/src/apex/install_nvhpc22.11/bin

rm -rf build_cuda

cmake -B build_cuda \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_INSTALL_PREFIX=`pwd`/install_cuda \
-DCUDAToolkit_ROOT=${CUDA} \
-DZeroSum_WITH_CUDA=TRUE \
-DZeroSum_STANDALONE=TRUE \

cmake --build build_cuda -j
cmake --install build_cuda
ctest --test-dir build_cuda --rerun-failed --output-on-failure

#-DCMAKE_CXX_COMPILER=`whic nvc++` \
#-DCMAKE_C_COMPILER=`which nvc` \
