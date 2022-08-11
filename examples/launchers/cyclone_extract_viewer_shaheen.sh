#!/bin/bash

INSHIMTU_SW_ROOT="/lustre/sw/vis/development/Inshimtu"
INSHIMTU_ROOT="/lustre/project/k1033/Development/Inshimtu"
INSHIMTU_DIR="$INSHIMTU_SW_ROOT/build.shaheen"


# NOTE: To launch directly (short sessions):
# salloc --partition=debug
# srun bash -i # srun -u --pty bash -i
# cd /lustre/project/k1033/Development/Inshimtu
# export INSHIMTU_CLIENT=<paraview_client_hostname>
# examples/cyclone_extract_viewer_shaheen.sh


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
    -i "${INSHIMTU_ROOT}/examples/testing/GDM/"wrfout_d01_* \
    -s "${INSHIMTU_ROOT}/examples/catalyst_extract_viewer_shaheen.py" \
    -v "P,U,V,W,QICE" \
    -n 0

