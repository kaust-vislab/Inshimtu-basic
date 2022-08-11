#!/bin/bash

# workflow.initialize.sh


## INPUTS

checkFunction checkDirectory
checkFunction checkFile
checkFunction checkExecutable
checkFunction checkNonEmpty
checkFunction checkVarSet
checkFunction checkNumber
checkFunction checkBoolean


checkDirectory WKFLOW_WORK_DIR
checkDirectory WKFLOW_PROJECT_DIR
checkNonEmpty WKFLOW_WORKDIR_INITIALIZE_REQ

checkNonEmpty SIM_OUTPUT_DIR


# MODULES

checkFile WRFRUN_MODULES
source "${WRFRUN_MODULES}"

checkDirectory WRF_DIR

WRFRUN_BASE_SRCDIR="${WRF_DIR}/run"
checkDirectory WRFRUN_BASE_SRCDIR


## SETUP

WRFRUN_BASE_DIR="${WKFLOW_WORK_DIR}/wrfrun-source"
WRFRUN_INIT_DIR="${WKFLOW_WORK_DIR}/wrfrun-initial"


## PREPARE

# Prepare simulation input files

case $WKFLOW_WORKDIR_INITIALIZE_REQ in
  copy)
    # TODO: use rsync to prevent unnecessary data transfer
    ;;
  link)
    \ln -snf "${WRFRUN_BASE_SRCDIR}/" "$WRFRUN_BASE_DIR"
    \ln -snf "${WKFLOW_PROJECT_DIR}/wrfrun-initial/" "$WRFRUN_INIT_DIR"
    ;;
  none)
    ;;
  *)
    echo "Invalid WKFLOW_WORKDIR_INITIALIZE_REQ: ${WKFLOW_WORKDIR_INITIALIZE_REQ}"
    exit 1
    ;;
esac


checkDirectory WRFRUN_BASE_DIR
checkDirectory WRFRUN_INIT_DIR


# BUILD SIMULATION DIR
# Create simulation output directory
mkdir -p "$SIM_OUTPUT_DIR"
cd "$SIM_OUTPUT_DIR"

# Populate simulation output directory
# NOTE: Require relative paths because absolute path can be specific to current job (i.e., changes each run)
find "${WRFRUN_BASE_DIR}/" -type f -name "*" -not -name "*.exe" -not -name "*.input" \
     -exec \ln -srnf {} . \;
find "${WRFRUN_INIT_DIR}/" \( -type f -or -type l \) -name "*" \
     -exec \ln -srnf {} . \;

# Update simulation output directory
rm -f rsl.error.* rsl.out.*
rm -f wrfrst_d* wrf-sur_d* wrfxtrm_d*
rm -f wrfout_d* wrfoutReady_d*

# Log simulation output directory
ls -alhFR "$SIM_OUTPUT_DIR"

######################################

