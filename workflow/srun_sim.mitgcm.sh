#!/bin/bash

# srun_mitgcm.sh


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
checkPositiveNumber SIM_TOTAL_TASKS

checkPositiveNumber INSHIMTU_INPORT_NODES_COUNT
checkPositiveNumber SLURM_NNODES

checkFile MITGCMRUN_MODULES


## PATHS

WORK_DIR="${WKFLOW_WORK_DIR}"


## PARAMETERS

MITGCMRUN_TOTAL_NODES=$(( SLURM_NNODES - INSHIMTU_INPORT_NODES_COUNT ))
MITGCMRUN_TOTAL_TASKS=$SIM_TOTAL_TASKS


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
export MPICH_MPIIO_HINTS="*:cb_nodes=40:,\
*.data:cb_nodes=40:striping_unit=2097152,\
*.meta:cb_nodes=40:striping_unit=1048576"
export MPICH_MPIIO_TIMERS=1


# MODULES

checkFile MITGCMRUN_MODULES
source "${MITGCMRUN_MODULES}"

checkDirectory MITGCM_DIR

MITGCM_EXEC="${MITGCM_DIR}/mitgcmuv"
checkExecutable MITGCM_EXEC


# SETUP
echo "Setup MITGCM configuration"

# Output Configuration
echo "Launching MITGCM"
echo "  MITGCM_EXEC=$MITGCM_EXEC"
echo "  SLURM_NNODES=$SLURM_NNODES"
echo "  MITGCMRUN_TOTAL_NODES=$MITGCMRUN_TOTAL_NODES"
echo "  MITGCMRUN_TOTAL_TASKS=$MITGCMRUN_TOTAL_TASKS"
echo "  SIM_OUTPUT_DIR=$SIM_OUTPUT_DIR"
echo "  SIM_DONE_FILE=$SIM_DONE_FILE"
echo "  WORK_DIR=$WORK_DIR"


# RUN - MITGCM

ulimit -s unlimited

time srun --chdir="$SIM_OUTPUT_DIR" \
          --nodes=$MITGCMRUN_TOTAL_NODES --ntasks=$MITGCMRUN_TOTAL_TASKS \
          "$MITGCM_EXEC"

