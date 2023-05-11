#!/bin/bash -e
#SBATCH -A phy122-ecp
#SBATCH -J test_xgc
#SBATCH -t 0:05:00
#SBATCH --threads-per-core=2 
#SBATCH -N 1
#SBATCH -q debug
#SBATCH -S 8

# Assumption - this script will reserve 1 core per L3 bank for system activity.

export OMP_PROC_BIND=spread
export OMP_PLACES=cores
export OMP_NUM_THREADS=14
export OMP_SCHEDULE=guided
export SLURM_CPU_BIND_VERBOSE=1

srun -n8 --cpus-per-task=14 --gpu-bind=closest ./build/bin/zerosum-mpi ./build/bin/lu-decomp-mpi
