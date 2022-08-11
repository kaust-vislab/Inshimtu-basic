#!/bin/bash

INSHIMTU_SW_ROOT="/lustre/sw/vis/development/Inshimtu"
EXAMPLES_DIR="/scratch/holstgr/tmp/wrf2pv-testing/examples"
DATA_DIR="/scratch/tmp/wrf-3.7.1/run_big/ouput"
INSHIMTU_DIR="$INSHIMTU_SW_ROOT/build.shaheen"

SRC_DIR="lustre/project/k1033/Development/Inshimtu"

DEST_DIR="/scratch/$USER/inshimtu-testing"

# NOTE: To launch directly (short sessions):
# salloc --partition=debug
# srun -u --pty bash -i
# cd $EXAMPLES_DIR
# ./cyclone_extract_writer_big_shaheen.sh


## SETUP
echo "Setup TestReadyFile configuration"

WORK_DIR="${DEST_DIR}/test-CycloneExtractWriterBig"
CONFIG_FILE="${WORK_DIR}/config.json"
DATA_DIR="${WORK_DIR}/data"
#COHELPER_SCRIPT_SH="${WORK_DIR}/doDelayedReady.sh"

mkdir -p "${WORK_DIR}"





# Load Modules

module use /lustre/sw/vis/modulefiles
module add Boost/1.61.0-CrayGNU-2016.07.KSL
module add ParaView/5.4.1-CrayGNU-2016.07.KSL-server-mesa
module add cray-hdf5-parallel/1.10.0.1


cd "${WORK_DIR}"

"$INSHIMTU_DIR/Inshimtu" -c "${CONFIG_FILE}"
    -i "$DATA_DIR/"wrfout_d01_* \
    -s "$EXAMPLES_DIR/catalyst_extract_writer_shaheen.py" \
    -v "P,U,V,W,QICE" \
    -n 0

