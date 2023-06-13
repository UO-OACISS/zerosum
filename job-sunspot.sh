#!/bin/bash
#PBS -A XGC_aesp_CNDA
####PBS -q workq
#PBS -q debug
#PBS -l select=1
#PBS -l walltime=10:00
#PBS -l filesystems=home
#PBS -e error.log
#PBS -o output.log

cd $PBS_O_WORKDIR

env | grep PBS_
env | grep ZE_

# set the number of threads based on --cpus-per-task
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PROC_BIND=close
# Bind OpenMP threads to hardware threads
export OMP_PLACES=threads
export OMP_NUM_THREADS=2

# 52 total cores per CPU
# 2 threads per core
#

mpiexec -n 6 -ppn 6 \
--cpu-bind=verbose,depth \
-d 24 \
-envall \
/soft/tools/mpi_wrapper_utils/gpu_tile_compact.sh \
./build/bin/zerosum-mpi \
./build/bin/lu-decomp-mpi

#./build/bin/random_walk 10000 1000000 1000

