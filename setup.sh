#!/bin/sh -e

export INSHIMTU_DIR=$(cd `dirname $0` && pwd)

OSVERSION=$(echo `lsb_release -sir` | awk -F '.' '{ print $1 }')
HOSTDOMAIN=$(hostname -d)
[[ -z "$HOSTDOMAIN" ]] && HOSTDOMAIN=$(hostname)


function buildShaheenCDL5 {

  INSHIMTU_PROFILING=${INSHIMTU_PROFILING:-false}
  if [ "$INSHIMTU_PROFILING" = true ] ; then
    LIB_EXT="so"
    BUILD_TYPE="RelWithDebInfo"
    export CRAYPE_LINK_TYPE=dynamic
  else
    LIB_EXT="so"
    BUILD_TYPE="Release"
    export CRAYPE_LINK_TYPE=dynamic
  fi


  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.shaheen"
  echo "Building on Shaheen cdl5"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"

  if [ "$INSHIMTU_PROFILING" = true ] ; then
    # Enable Cray profiling tools
    module unload darshan
    module load perftools-base
    # TODO: choose type automatically
    # Choose one: simple sampling, simple events based, advanced
    #module load perftools-lite
    #module load perftools-lite-events
    module load perftools

    # ARM Allinea support
    module load arm-reports/19.1.4 arm-forge/19.1.4
    # ARM Allinea support
    make-profiler-libraries
  fi

  export CRAYPE_LINK_TYPE=dynamic

  export ParaView_DIR=/project/k1033/kressjm/shaheen/inshimtu-env/paraview/build/install/lib/cmake


  cmake -DCMAKE_SYSTEM_NAME=CrayLinuxEnvironment -DBoost_INCLUDE_DIR="/project/k1033/kressjm/shaheen/inshimtu-env/boost_1_67_0"  -DCMAKE_BUILD_TYPE="RelWithDebInfo" \
        -DCMAKE_C_COMPILER="$(which cc)" -DCMAKE_CXX_COMPILER="$(which CC)" \
        -DCMAKE_C_FLAGS="-g -fopenmp" -DCMAKE_CXX_FLAGS="-g -fopenmp" \
        ..

 #make -j4
# TODO: enable VERBOSE in debug mode
#  VERBOSE=1 make -j 12
#  VERBOSE=1 make -j 12 Inshimtu
#  VERBOSE=1 make -j 12 InshimtuLib


  if [ "$INSHIMTU_PROFILING" = true ] ; then
    # TODO: pat_build only needed for perftools
    echo "pat_build Inshimtu -- for advanced perftools"
    pat_build Inshimtu
  fi
}

function buildShaheen {
  # NOTE: Build on compute node:
  # salloc --partition=debug
  # srun -u --pty bash -i
  # cd /sw/vis/development/Inshimtu
  # ./setup.sh

  INSHIMTU_PROFILING=${INSHIMTU_PROFILING:-false}
  if [ "$INSHIMTU_PROFILING" = true ] ; then
    LIB_EXT="so"
    BUILD_TYPE="RelWithDebInfo"
    export CRAYPE_LINK_TYPE=dynamic
  else
    LIB_EXT="so"
    BUILD_TYPE="Release"
    export CRAYPE_LINK_TYPE=dynamic
  fi


  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.shaheen"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"


  module add cmake

  ## NOTE: `module add ParaView/5.8.0-CrayGNU-2019.12.KSL-server-mesa`: 
  #        uses Python 3.7, but old Boost built against Python 2.7; there is a linker issue as a result
  ## NOTE: `module add ParaView/5.8.0-CrayGNU-2019.12.KSL-server-mesa-py27`: 
  #        builds; testing... 
  echo "Creating Module File"
cat <<'EOF' > "${INSHIMTU_BUILD_DIR}/module.init"
  module load cray-python/3.9.4.1 
  module load cray-netcdf/4.7.4.4 
  module load cray-hdf5/1.12.0.4 
EOF
  source "${INSHIMTU_BUILD_DIR}/module.init"

  if [ "$INSHIMTU_PROFILING" = true ] ; then
    # Enable Cray profiling tools
    module unload darshan
    module load perftools-base
    # TODO: choose type automatically
    # Choose one: simple sampling, simple events based, advanced
    #module load perftools-lite
    #module load perftools-lite-events
    module load perftools

    # ARM Allinea support
    module load arm-reports/19.1.4 arm-forge/19.1.4
    # ARM Allinea support
    make-profiler-libraries
  fi

  export CRAYPE_LINK_TYPE=dynamic

  export ParaView_DIR=/lustre/scratch/x_esposia/KVL/ParaView-v5.9.1.install/


  cmake -DCMAKE_SYSTEM_NAME=CrayLinuxEnvironment -DBoost_INCLUDE_DIR="/lustre/scratch/x_esposia/KVL/boost_1_67_0" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_C_COMPILER="$(which cc)" -DCMAKE_CXX_COMPILER="$(which CC)" \
        -DCMAKE_C_FLAGS="-g -fopenmp" -DCMAKE_CXX_FLAGS="-g -fopenmp" \
        ..

  make -j4
# TODO: enable VERBOSE in debug mode
#  VERBOSE=1 make -j 12
#  VERBOSE=1 make -j 12 Inshimtu
#  VERBOSE=1 make -j 12 InshimtuLib


  if [ "$INSHIMTU_PROFILING" = true ] ; then
    # TODO: pat_build only needed for perftools
    echo "pat_build Inshimtu -- for advanced perftools"
    pat_build Inshimtu
  fi
}

function buildIbex {
  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.ibex"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"

  BUILD_TYPE="Release"

  echo "Creating Module File"
cat <<'EOF' > "${INSHIMTU_BUILD_DIR}/module.init"
  module use /sw/vis/ibex-gpu.modules
  module add CMake/3.12.1
  module add ParaView/5.6.0-openmpi-x86_64
EOF
  source "${INSHIMTU_BUILD_DIR}/module.init"

  cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DBOOST_ROOT="${BOOST_DIR}" ..

  make -j 8
}

function buildLocal {
  echo "Executing local build"
  LIB_EXT="so"
  BUILD_TYPE="RelWithDebInfo"

  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.local"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"


  export ParaView_DIR="/home/kressjm/packages/inshimtu-basic/paraview-build/install"

  cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DBoost_INCLUDE_DIR="/home/kressjm/packages/inshimtu-basic/boost_1_67_0" \
        ..

  make -j 12
}


# Determine Build Recipe
case "$HOSTDOMAIN" in
*"jupiter")
  buildDC
  ;;
*"ibex.kaust.edu.sa")
  buildIbex
  ;;
*"hpc.kaust.edu.sa")
  case "$OSVERSION" in
  "SUSE"*)
    buildShaheenCDL5
    ;;
  *)
    echo "Unknown build environment."
    exit 1
    ;;
  esac
  ;;
*)
  case "$OSVERSION" in
  "SUSE"*)
    # host domain not available on Cray compute nodes
    buildShaheen
    ;;
  "Ubuntu"*)
    buildLocal
    ;;
  *)
    echo "Unknown build environment. Unknown system domain."
    exit 1
    ;;
  esac
  echo "Unknown build environment"
  exit 1
  ;;
esac


