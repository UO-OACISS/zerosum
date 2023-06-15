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
echo Jobid: $PBS_JOBID
echo Running on host `hostname`
echo Running on nodes `cat $PBS_NODEFILE`

env | grep PBS_
env | grep ZE_

# set the number of threads based on --cpus-per-task
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PROC_BIND=close
# Bind OpenMP threads to cores
export OMP_PLACES=cores
export OMP_NUM_THREADS=8

NNODES=1
export RANKS_PER_NODE=12           # Number of MPI ranks per node
NRANKS=$(( NNODES * RANKS_PER_NODE ))

echo "NUM_OF_NODES=${NNODES}  TOTAL_NUM_RANKS=${NRANKS}  RANKS_PER_NODE=${RANKS_PER_NODE}"

if ((PALS_LOCAL_RANKID==0)); then
  export ZE_AFFINITY_MASK=0.0
fi

if ((PALS_LOCAL_RANKID==1)); then
  export ZE_AFFINITY_MASK=0.1
fi

if ((PALS_LOCAL_RANKID==2)); then
  export ZE_AFFINITY_MASK=1.0
fi

if ((PALS_LOCAL_RANKID==3)); then
  export ZE_AFFINITY_MASK=1.1
fi

if ((PALS_LOCAL_RANKID==4)); then
  export ZE_AFFINITY_MASK=2.0
fi

if ((PALS_LOCAL_RANKID==5)); then
  export ZE_AFFINITY_MASK=2.1
fi

if ((PALS_LOCAL_RANKID==6)); then
  export ZE_AFFINITY_MASK=3.0
fi

if ((PALS_LOCAL_RANKID==7)); then
  export ZE_AFFINITY_MASK=3.1
fi

if ((PALS_LOCAL_RANKID==8)); then
  export ZE_AFFINITY_MASK=4.0
fi

if ((PALS_LOCAL_RANKID==9)); then
  export ZE_AFFINITY_MASK=4.1
fi

if ((PALS_LOCAL_RANKID==10)); then
  export ZE_AFFINITY_MASK=5.0
fi

if ((PALS_LOCAL_RANKID==11)); then
  export ZE_AFFINITY_MASK=5.1
fi

# This enables the ability to query SYCL for free GPU memory
export ZES_ENABLE_SYSMAN=1

mpiexec --np ${NRANKS} -ppn ${RANKS_PER_NODE} \
--cpu-bind verbose,depth -d 16 \
-envall \
/soft/tools/mpi_wrapper_utils/gpu_tile_compact.sh \
./build/bin/zerosum-mpi \
./build/bin/lu-decomp-mpi

#--cpu-bind verbose,list:0-7:8-15:16-25:26-33:34-41:42-51:52-59:60-67:68-77:78-85:86-93:94-103 \
#./build/bin/random_walk 10000 1000000 1000

