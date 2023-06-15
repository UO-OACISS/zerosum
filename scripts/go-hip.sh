module load openmpi/4.1.1-rocm5.2.0

PATH=$PATH:$HOME/src/apex/install_gilgamesh_5.2.0/bin

rm -rf build_hip

cmake -B build_hip \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_CXX_COMPILER=`which amdclang++` \
-DCMAKE_C_COMPILER=`which amdclang` \
-DCMAKE_INSTALL_PREFIX=`pwd`/install_hip \
-DCMAKE_C_FLAGS=-gdwarf-4 \
-DCMAKE_CXX_FLAGS=-gdwarf-4 \
-DROCM_ROOT=/opt/rocm-5.2.0 \
-DZeroSum_WITH_HIP=TRUE \
-DZeroSum_STANDALONE=TRUE \

cmake --build build_hip -j
cmake --install build_hip
ctest --test-dir build_hip

#-DZeroSum_WITH_PerfStubs=FALSE \


