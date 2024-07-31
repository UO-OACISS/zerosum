module reset
module unload perftools-base
module load cmake
module load PrgEnv-amd
module swap amd amd/5.7.1
module load rocm/5.7.1
module swap cray-mpich cray-mpich/8.1.28
module load craype-accel-amd-gfx90a
module unload darshan-runtime
export CRAYPE_LINK_TYPE=dynamic
export OMP_PROC_BIND=true
export OMP_PLACES=threads
export OMP_NUM_THREADS=7
