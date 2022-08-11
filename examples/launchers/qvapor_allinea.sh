#!/bin/bash

INSHIMTU_ROOT="/lustre/project/k1033/Development/Inshimtu"
INSHIMTU_DIR="$INSHIMTU_ROOT/build.shaheen"


# Load Modules

# Enable Cray profiling tools
module unload darshan

module swap PrgEnv-cray PrgEnv-gnu
module add cdt/16.07

module use /lustre/sw/vis/modulefiles
module add Boost/1.61.0-CrayGNU-2016.07.KSL
module add ParaView/5.3.0-CrayGNU-2016.07.KSL-server-mesa
module add cray-hdf5-parallel/1.10.0.1

# Enable Cray profiling tools and Allinea support
module load perftools-base perftools
module load allinea-reports/7.0 allinea-forge/7.0


cd "$INSHIMTU_ROOT/examples"

"$INSHIMTU_DIR/Inshimtu" \
    -w "${INSHIMTU_ROOT}/examples/testing/GDM" \
    -d "${INSHIMTU_ROOT}/examples/testing.done" \
    -i "${INSHIMTU_ROOT}/examples/testing/GDM/"wrfout_d01_* \
    -s "${INSHIMTU_ROOT}/examples/pipelines/gridwriter_QVAPOR.py" -v QVAPOR

