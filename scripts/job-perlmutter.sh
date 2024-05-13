#!/bin/bash
#SBATCH --account=m499_g
#SBATCH --qos=regular
#SBATCH --constraint=gpu
#SBATCH --job-name=n531pe
#SBATCH --nodes=1
#SBATCH --time=0:05:00
#SBATCH --mail-type=BEGIN,END,FAIL
#SBATCH --mail-user=khuck@cs.uoregon.edu

export OMP_NUM_THREADS=4
export OMP_PROC_BIND=spread
export OMP_PLACES=threads
#export OMP_STACKSIZE=2G   # required for GNU build to prevent a segfault
#export MPICH_GPU_SUPPORT_ENABLED=1

export n_mpi_ranks_per_node=4
export n_mpi_ranks=$((${SLURM_JOB_NUM_NODES} * ${n_mpi_ranks_per_node}))
echo 'Number of nodes: '                  ${SLURM_JOB_NUM_NODES}
echo 'MPI ranks (total): '                $n_mpi_ranks
echo 'MPI ranks per node: '               $n_mpi_ranks_per_node
echo 'Number of OMP threads: '            ${OMP_NUM_THREADS}
echo 'XGC executable: '                   ${XGC_BIN}
echo ''

set -x
srun --unbuffered -N ${SLURM_JOB_NUM_NODES} -n ${n_mpi_ranks} \
-c ${OMP_NUM_THREADS} --cpu-bind=verbose,threads \
--ntasks-per-node=${n_mpi_ranks_per_node} --gpus-per-task=1 --gpu-bind=single:1 \
$SCRATCH/zerosum/bin/zerosum-mpi --zs:verbose ./build/bin/lu-decomp-mpi
set +x
