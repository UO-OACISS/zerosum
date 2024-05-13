#!/bin/bash -l
#PBS -N zerosum
#PBS -l select=1:system=polaris
#PBS -l walltime=0:05:00
#PBS -q debug
#PBS -A Tools
#PBS -l filesystems=home


NNODES=`wc -l < $PBS_NODEFILE`
NRANKS=4 # Number of MPI ranks to spawn per node
NDEPTH=8 # Number of hardware threads per rank (i.e. spacing between MPI ranks)
NTHREADS=8 # Number of software threads per rank to launch (i.e. OMP_NUM_THREADS)

NTOTRANKS=$(( NNODES * NRANKS ))

echo "NUM_OF_NODES= ${NNODES} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS} THREADS_PER_RANK= ${NTHREADS}"

cd $HOME/src/zerosum
source ./scripts/sourceme-polaris-sycl.sh
source ./scripts/sourceme-common.sh

mpiexec --np ${NTOTRANKS} -ppn ${NRANKS} -d ${NDEPTH} --cpu-bind depth -env OMP_NUM_THREADS=${NTHREADS} ./build/bin/zerosum-mpi ./build/bin/lu-decomp-mpi
