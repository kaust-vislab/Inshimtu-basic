#!/bin/bash

# srun_inshimtu.sh


## INPUTS

checkFunction checkDirectory
checkFunction checkFile
checkFunction checkExecutable
checkFunction checkNonEmpty
checkFunction checkVarSet
checkFunction checkPositiveNumber
checkFunction checkNumber


checkDirectory WKFLOW_WORK_DIR
checkDirectory SIM_OUTPUT_DIR
checkFile SIM_DONE_FILE

checkPositiveNumber INSHIMTU_INPORT_NODES_COUNT
checkPositiveNumber SLURM_NNODES

checkDirectory INSHIMTU_BUILD_DIR
checkVarSet INSHIMTU_FLAGS

checkDirectory INSHIMTU_PIPELINE_DIR
checkFile INSHIMTU_CONFIG_FILE
checkDirectory INSHIMTU_RESULTS_DIR


## PATHS


INSHIMTU_EXEC="${INSHIMTU_BUILD_DIR}/Inshimtu"
checkExecutable INSHIMTU_EXEC
INSHIMTU_MODULES="${INSHIMTU_BUILD_DIR}/module.init"
checkFile INSHIMTU_MODULES

WORK_DIR="${WKFLOW_WORK_DIR}"

FILTERS_DIR="${INSHIMTU_PIPELINE_DIR}"


## PARAMETERS

INSHIMTU_INPORT_NODES_START=$(( SLURM_NNODES - INSHIMTU_INPORT_NODES_COUNT ))
INSHIMTU_INPORT_NODES_END=$(( INSHIMTU_INPORT_NODES_START + INSHIMTU_INPORT_NODES_COUNT - 1 ))
INSHIMTU_INPORT_NODES="${INSHIMTU_INPORT_NODES_START}-${INSHIMTU_INPORT_NODES_END}"


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


## SETUP
echo "Setup Inshimtu configuration"

export INSHIMTU_FILTER_SCRIPT_DIR="${FILTERS_DIR}"
export INSHIMTU_WRITE_OUTPUT_DIR="${INSHIMTU_RESULTS_DIR}"


# Output Configuration
echo "Launching WRF+Inshimtu"
echo "  INSHIMTU_EXEC=$INSHIMTU_EXEC"
echo "  SLURM_NNODES=$SLURM_NNODES"
echo "  INSHIMTU_INPORT_NODES=$INSHIMTU_INPORT_NODES"
echo "  SIM_OUTPUT_DIR=$SIM_OUTPUT_DIR"
echo "  SIM_DONE_FILE=$SIM_DONE_FILE"
echo "  WORK_DIR=$WORK_DIR"
echo "  INSHIMTU_FILTER_SCRIPT_DIR=$INSHIMTU_FILTER_SCRIPT_DIR"
echo "  INSHIMTU_WRITE_OUTPUT_DIR=$INSHIMTU_WRITE_OUTPUT_DIR"
echo "  SSH_CLIENT IP: ${SSH_CLIENT%% *}"
echo "  INSHIMTU_CONFIG_FILE=$INSHIMTU_CONFIG_FILE"
echo "    Configuration: $(cat "$INSHIMTU_CONFIG_FILE")"


# MODULES - Inshimtu

checkFile INSHIMTU_MODULES
source "${INSHIMTU_MODULES}"

# RUN - Inshimtu

checkExecutable INSHIMTU_EXEC

# Launch Inshimtu (background)
# Note: Override config file for data paths, inport nodes, and file deletion
srun --ntasks-per-node=1 --chdir="${WORK_DIR}" \
     "$INSHIMTU_EXEC" -c "${INSHIMTU_CONFIG_FILE}" \
                      -w "${SIM_OUTPUT_DIR}" \
                      -d "${SIM_DONE_FILE}" \
                      -n "${INSHIMTU_INPORT_NODES}" \
                      ${INSHIMTU_FLAGS} \
  &

# wait for ready
sleep 60

