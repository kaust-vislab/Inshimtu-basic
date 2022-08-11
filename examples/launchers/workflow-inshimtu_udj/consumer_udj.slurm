#!/bin/bash

#SBATCH -A k1033
#SBATCH -N 1
#SBATCH --exclusive
#SBATCH --hint=nomultithread
#SBATCH -c 1
#SBATCH -t 60

WRFRUN_TOTAL_TASKS=4
CONSUMER_TOTAL_TASKS=4
CONSUMER_NODES=1

# MPI IO Directives
# Displays all settings used by the MPI during execution
#export MPICH_ENV_DISPLAY=1
# Displays MPI version
#export MPICH_VERSION_DISPLAY=1
# Display ranks performing IO aggregation when using collective buffering
#export MPICH_MPIIO_AGGREGATOR_PLACEMENT_DISPLAY=1
# Display read/write operations statistics after collective buffering
export MPICH_MPIIO_STATS=1
#Displays all the available I/O hints and their values
export MPICH_MPIIO_HINTS_DISPLAY=1
# Set per-file striping (lustre / burst buffer)
export MPICH_MPIIO_HINTS="wrfrst*:cb_nodes=40:,\
wrfout*:cb_nodes=40:striping_unit=2097152,\
wrfi*:cb_nodes=40:striping_unit=1048576"
export MPICH_MPIIO_TIMERS=1
export MPICH_MAX_THREAD_SAFETY=multiple
export ATP_ENABLED=0
#ulimit -c unlimited

CONSUMER_EXEC="env CONSUMER=1 /lustre/scratch/x_esposia/UDJ/test_driver/consumer_kvl_wrf_udj.exe"

ulimit -s unlimited
export OMP_STACKSIZE=64m
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

#env > env.out

####################################################################################################
# UDJ specific information 
export PMI_USE_DRC=1
export UDJ_TRANSPORT_ORDER=MPI
export UDJ_WORKFLOW_ID=mytest_for_wrf
export UDJT_MPI_USE_DPM="yes"
export MPICH_DPM_DIR=/lustre/scratch/x_esposia/DPM
export UDJ_CONSENSUS_DIR=/lustre/scratch/x_esposia/UDJ_CONSENSUS
rm -f ${MPICH_DPM_DIR}/.mpidpm/contacter
rm -rf ${UDJ_CONSENSUS_DIR}/${UDJ_WORKFLOW_ID}*
sleep 3
#export UDJ_TRACE_ARGS=detail
export UDJ_LOG_LEVEL=1
export UDJT_MPI_INIT_WAIT_CONNECTED=1

####################################################################################################
# WRF-UDJ specific information
export COMM_UDJ_MODE=DPM
export CONSUMER_UDJ_RANKS="0-127" # has to be a subset of [0,,total_ranks_in_srun - 1]
export TWODIM_UDJ_SEND_PROC_DIMS="8,16"
export TWODIM_UDJ_RECV_PROC_DIMS="8,16"
export THREEDIM_UDJ_SEND_PROC_DIMS="8,16,1"
export THREEDIM_UDJ_RECV_PROC_DIMS="8,16,1"

time srun -l --unbuffered -n ${CONSUMER_TOTAL_TASKS} -N ${CONSUMER_NODES} ${CONSUMER_EXEC}



