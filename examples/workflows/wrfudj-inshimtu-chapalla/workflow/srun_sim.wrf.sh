#!/bin/bash

# srun_wrf.sh


## INPUTS

checkFunction checkDirectory
checkFunction checkFile
checkFunction checkExecutable
checkFunction checkNonEmpty
checkFunction checkVarSet
checkFunction checkNumber
checkFunction checkPositiveNumber


checkDirectory WKFLOW_WORK_DIR
checkDirectory SIM_OUTPUT_DIR

checkPositiveNumber SLURM_NNODES

checkFile WRFRUN_MODULES


## PATHS

WORK_DIR="${WKFLOW_WORK_DIR}"


## PARAMETERS

INSITU_NODES_COUNT=1
WRFRUN_TASKS_PER_NODE=16
CONSUMER_TASKS_PER_NODE=16

WRFRUN_TOTAL_NODES=$(( SLURM_NNODES - INSITU_NODES_COUNT ))
WRFRUN_TOTAL_TASKS=$(( WRFRUN_TOTAL_NODES * WRFRUN_TASKS_PER_NODE ))

CONSUMER_TOTAL_TASKS=$(( INSITU_NODES_COUNT * CONSUMER_TASKS_PER_NODE ))

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
export FOR_IGNORE_EXCEPTIONS=true
export ATP_ENABLED=0

####################################################################################################
# UDJ specific information 
export UDJ_TRANSPORT_ORDER=MPI
export UDJ_WORKFLOW_ID=mytest_for_wrf
export UDJT_MPI_USE_DPM="no"
#export MPICH_DPM_DIR=/lustre/scratch/x_esposia/DPM
#export UDJ_CONSENSUS_DIR=/lustre/scratch/x_esposia/UDJ_CONSENSUS
#export UDJ_TRACE_ARGS=detail
export UDJ_LOG_LEVEL=1
export UDJT_MPI_INIT_WAIT_CONNECTED=1
#export MAX_NTRANSFERS=100
export FILTER_VARNAME="U"

####################################################################################################
# WRF-UDJ specific information
export TWODIM_UDJ_SEND_PROC_DIMS="16,12"
export TWODIM_UDJ_RECV_PROC_DIMS="4,4"
export THREEDIM_UDJ_SEND_PROC_DIMS="16,12,1"
export THREEDIM_UDJ_RECV_PROC_DIMS="4,4,1"

# choose between communicaton mode.
#export COMM_UDJ_MODE=DPM
export COMM_UDJ_MODE=MPMD

ulimit -s unlimited
export OMP_STACKSIZE=64m
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

# MODULES

checkFile WRFRUN_MODULES
source "${WRFRUN_MODULES}"

checkDirectory WRF_DIR

WRF_EXEC="${WRF_DIR}/run/wrf.exe"
checkExecutable WRF_EXEC
CONSUMER_EXEC="/project/k1033/Development/Inshimtu/build.shaheen/Inshimtu"
checkExecutable CONSUMER_EXEC

# SETUP
echo "Setup WRF configuration"

# Output Configuration
echo "Launching WRF"
echo "  WRF_EXEC=$WRF_EXEC"
echo "  SLURM_NNODES=$SLURM_NNODES"
echo "  WRFRUN_TASKS_PER_NODE=$WRFRUN_TASKS_PER_NODE"
echo "  WRFRUN_TOTAL_NODES=$WRFRUN_TOTAL_NODES"
echo "  WRFRUN_TOTAL_TASKS=$WRFRUN_TOTAL_TASKS"
echo "  SIM_OUTPUT_DIR=$SIM_OUTPUT_DIR"
echo "  WORK_DIR=$WORK_DIR"


# RUN - WRF

case $COMM_UDJ_MODE in
DPM)
    export PMI_USE_DRC=1
    export PRODUCER_UDJ_RANKS="0-$((WRFRUN_TOTAL_TASKS-1))"

    echo PRODUCER_UDJ_RANKS="$PRODUCER_UDJ_RANKS"

    time srun --chdir="$SIM_OUTPUT_DIR" --label --unbuffered \
              --ntasks ${WRFRUN_TOTAL_TASKS} --nodes ${WRFRUN_TOTAL_NODES} \
              env PRODUCER=1 ${WRF_EXEC}
    ;;
MPMD)
    PRODUCER_RANKS="0-$((WRFRUN_TOTAL_TASKS-1))"
    CONSUMER_RANKS="${WRFRUN_TOTAL_TASKS}-$((WRFRUN_TOTAL_TASKS + CONSUMER_TOTAL_TASKS - 1))"
    echo PRODUCER_RANKS="$PRODUCER_RANKS"
    echo CONSUMER_RANKS="$CONSUMER_RANKS"

    export PRODUCER_UDJ_RANKS=${PRODUCER_RANKS}
    export CONSUMER_UDJ_RANKS=${CONSUMER_RANKS}
    echo PRODUCER_UDJ_RANKS="$PRODUCER_UDJ_RANKS"
    echo CONSUMER_UDJ_RANKS="$CONSUMER_UDJ_RANKS"

    LAUNCH_CONFIG="${WORK_ROOT}/launch.conf"
    echo "${PRODUCER_RANKS} env PRODUCER=1 ${WRF_EXE} " > "$LAUNCH_CONFIG"
    echo "${CONSUMER_RANKS} env LD_PRELOAD=/opt/cray/pe/lib64/libmpich_gnu_82.so.3 ${INSHIMTU_EXE} --udj-mode=mpmd -n ${CONSUMER_RANKS} -N ${CONSUMER_RANKS} -P ${PRODUCER_RANKS} -V trace -c ${INSHIMTU_CONFIG_FILE} --udj-process-dims-producer=${UDJ_SEND_PROC_DIMS} --process-dims-inporter=${UDJ_RECV_PROC_DIMS}" >> "$LAUNCH_CONFIG"

    srun --chdir="$SIM_OUTPUT_DIR" \
         --ntasks $((WRFRUN_TOTAL_TASKS + CONSUMER_TOTAL_TASKS)) \
         --label --unbuffered --multi-prog "$LAUNCH_CONFIG"
    ;;
?)
    echo "You have to specify COMM_UDJ_MODE"
    exit 1
    ;;
esac

