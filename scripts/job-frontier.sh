#!/bin/bash -e
#SBATCH -A phy122-ecp
#SBATCH -J test_xgc
#SBATCH -t 0:05:00
#SBATCH --threads-per-core=2 
#SBATCH -N 1
#SBATCH -q debug
#SBATCH -S 8

# Assumption - this script will reserve 1 core per L3 bank for system activity.
#module load rocm/5.2.0
cd $HOME/ECP-WDM/ecp-wdm-coe
source frontier.sh
cd $HOME/src/zerosum

unset OMP_PROC_BIND
unset OMP_PLACES
unset OMP_NUM_THREADS
unset OMP_SCHEDULE
export SLURM_CPU_BIND_VERBOSE=1

dosrun() {
    srun -n8 --gpus-per-task=1 --cpus-per-task=14 --gpu-bind=closest ./build.5.2.0/bin/zerosum-mpi ./build.5.2.0/bin/lu-decomp-mpi
}

echo "Defaults"
dosrun
for i in {0..7} ; do
    mv zs.${i}.log zs.14.default.${i}.log
done

echo "With Cores"
export OMP_PROC_BIND=close
export OMP_PLACES=cores
export OMP_NUM_THREADS=14
export OMP_SCHEDULE=dynamic
dosrun
for i in {0..7} ; do
    mv zs.${i}.log zs.14.cores.${i}.log
done

echo "With Threads"
export OMP_PROC_BIND=close
export OMP_PLACES=threads
export OMP_NUM_THREADS=14
dosrun
for i in {0..7} ; do
    mv zs.${i}.log zs.14.threads.${i}.log
done

