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
checkFile SIM_DONE_FILE

checkPositiveNumber INSHIMTU_INPORT_NODES_COUNT
checkPositiveNumber SLURM_NNODES

checkFile WRFRUN_MODULES


## PATHS

WORK_DIR="${WKFLOW_WORK_DIR}"


## PARAMETERS

WRFRUN_TASKS_PER_NODE=4

WRFRUN_TOTAL_NODES=$(( SLURM_NNODES - INSHIMTU_INPORT_NODES_COUNT ))
WRFRUN_TOTAL_TASKS=$(( WRFRUN_TOTAL_NODES * WRFRUN_TASKS_PER_NODE ))


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


# MODULES

checkFile WRFRUN_MODULES
source "${WRFRUN_MODULES}"

checkDirectory WRF_DIR

WRF_EXEC="${WRF_DIR}/run/wrf.exe"
checkExecutable WRF_EXEC


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
echo "  SIM_DONE_FILE=$SIM_DONE_FILE"
echo "  WORK_DIR=$WORK_DIR"


# RUN - WRF

ulimit -s unlimited
export OMP_STACKSIZE=64m
export OMP_NUM_THREADS=7

time srun --chdir="$SIM_OUTPUT_DIR" \
          --nodes=$WRFRUN_TOTAL_NODES \
          --ntasks=$WRFRUN_TOTAL_TASKS --ntasks-per-node=$WRFRUN_TASKS_PER_NODE \
          --cpus-per-task=7 --threads-per-core=1 --hint=nomultithread \
          "$WRF_EXEC"

