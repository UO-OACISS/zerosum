#!/bin/bash
#PBS -A XGC_aesp_CNDA
####PBS -q workq
#PBS -q debug
#PBS -l select=1
#PBS -l walltime=05:00
#PBS -l filesystems=home
#PBS -e error.log
#PBS -o output.log

cd $PBS_O_WORKDIR
echo Workdir: $PBS_O_WORKDIR
echo Jobid: $PBS_JOBID
echo Running on host `hostname`
echo Running on nodes `cat $PBS_NODEFILE`
#source scripts/sourceme-sunspot.sh
source scripts/sourceme-common.sh

# set the number of threads based on --cpus-per-task
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
export OMP_PROC_BIND=close
# Bind OpenMP threads to cores
export OMP_PLACES=threads

# For 12 processes per node:
export OMP_NUM_THREADS=16
export RANKS_PER_NODE=12           # Number of MPI ranks per node
export SCRIPT_NAME=gpu_tile_compact.sh

# For 6 processes per node:
#export OMP_NUM_THREADS=32
#export RANKS_PER_NODE=6           # Number of MPI ranks per node
#export SCRIPT_NAME=gpu_dev_compact.sh

NNODES=1
NRANKS=$(( NNODES * RANKS_PER_NODE ))

# This enables the ability to query SYCL for free GPU memory
export ZES_ENABLE_SYSMAN=1

# The complicated bind policy ensures that we skip core 0, and
# we assign 8 cores per process. There are 52 cores per socket.
# We have 12 processes, 16 threads per process. The threads are
# pinned to a core. Optionally, they could be pinned to a HW thread,
# as there are 2 HW threads per core. The bind list is HW threads IDs.
# --cpu-bind verbose,list:2-17:18-33:34-49:50-65:66-81:82-97:106-121:122-137:138-153:154-169:170-185:186-201 \

let nthreads=${OMP_NUM_THREADS}
mylist=""
let nthreads=8

let first_hwthread=2
let first_core=${first_hwthread}
let last_core=${first_core}+${nthreads}-1
let first_core2=106
let last_core2=${first_core2}+${nthreads}-1
let first_half=${NRANKS}/2

for i in $(seq 1 $first_half) ; do
    mylist="${mylist}:${first_core}-${last_core},${first_core2}-${last_core2}"
    let first_core=${first_core}+${nthreads}
    let first_core2=${first_core2}+${nthreads}
    let last_core=${last_core}+${nthreads}
    let last_core2=${last_core2}+${nthreads}
done

let first_hwthread=54
let first_core=${first_hwthread}
let last_core=${first_core}+${nthreads}-1
let first_core2=158
let last_core2=${first_core2}+${nthreads}-1

for i in $(seq 1 $first_half) ; do
    mylist="${mylist}:${first_core}-${last_core},${first_core2}-${last_core2}"
    let first_core=${first_core}+${nthreads}
    let first_core2=${first_core2}+${nthreads}
    let last_core=${last_core}+${nthreads}
    let last_core2=${last_core2}+${nthreads}
done

#export OMP_PROC_BIND=spread
#export OMP_NUM_THREADS=16
#unset OMP_PLACES
#mylist=":0-7,104-111:8-15,112-119:16-23,120-127:24-31,128-135:32-39,136-143:40-47,144-151:52-59,156-163:60-67,164-171:68-75,172-179:76-83,180-187:84-91,188-195:92-99,196-203"
echo ${mylist}

# For debugging
#export ZET_ENABLE_PROGRAM_DEBUGGING=1
#export INTELGT_AUTO_ATTACH_DISABLE=1

set -x
INTELGT_AUTO_ATTACH_DISABLE=1 mpiexec --np ${NRANKS} -ppn ${RANKS_PER_NODE} \
--cpu-bind verbose,list${mylist} \
-envall \
/soft/tools/mpi_wrapper_utils/${SCRIPT_NAME} \
./install/bin/zerosum-mpi \
--zs:logging \
./build/bin/lu-decomp-mpi 10
set +x

#--cpu-bind verbose,list:1-8:9-16:17-24:25-32:33-40:41-48:53-60:61-68:69-76:77-84:85-92:93-100 \
#--cpu-bind verbose,depth -d 16 \
#--cpu-bind verbose,list:0-7:8-15:16-25:26-33:34-41:42-51:52-59:60-67:68-77:78-85:86-93:94-103 \
#./build/bin/random_walk 10000 1000000 1000

