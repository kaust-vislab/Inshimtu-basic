#!/bin/sh -e

export INSHIMTU_DIR=$(cd `dirname $0` && pwd)

OSVERSION=$(echo `lsb_release -sir` | awk -F '.' '{ print $1 }')
HOSTDOMAIN=$(hostname -d)


function buildShaheen {
  # NOTE: Build on compute node:
  # salloc --partition=debug
  # srun -u --pty bash -i
  # cd /sw/vis/development/Inshimtu
  # ./setup.sh

  INSHIMTU_PROFILING=${INSHIMTU_PROFILING:-true}
  if [ "$INSHIMTU_PROFILING" = true ] ; then
    LIB_EXT="so"
    BUILD_TYPE="RelWithDebInfo"
  else
    LIB_EXT="so"
    BUILD_TYPE="Release"
  fi


  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.shaheen"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"


  module add cmake/3.13.4

  ## NOTE: `module add ParaView/5.8.0-CrayGNU-2019.12.KSL-server-mesa`: 
  #        uses Python 3.7, but old Boost built against Python 2.7; there is a linker issue as a result
  ## NOTE: `module add ParaView/5.8.0-CrayGNU-2019.12.KSL-server-mesa-py27`: 
  #        builds; testing... 
  ## NOTE: `module add ParaView/5.6.3-CrayGNU-2019.12.KSL-server-mesa`: 
  #        fails to find a "ParaView*config.cmake" file for Catalyst 
  ## NOTE: `module add ParaView/5.4.1-CrayGNU-2019.12.KSL-server-mesa`:
  #        needs older cmake code path
  echo "Creating Module File"
cat <<'EOF' > "${INSHIMTU_BUILD_DIR}/module.init"
  module use /sw/vis/xc40.modules
  module add ParaView/5.8.0-CrayGNU-2019.12.KSL-server-mesa-py27
  #module add cray-netcdf-hdf5parallel/4.6.3.2
  module add cray-parallel-netcdf/1.11.1.1
  module add cray-hdf5-parallel/1.10.5.2
  # TODO: required?
  export CRAYPE_LINK_TYPE=dynamic
  # TODO: Put fix in FFmpeg module
  export FFMPEG_ROOT="$(realpath "$(dirname "$(which ffmpeg)")"/..)"
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

  export ParaView_DIR="$(dirname "$(which pvserver)")"/../easybuild_obj

  cmake -DCMAKE_SYSTEM_NAME=CrayLinuxEnvironment -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_C_COMPILER="$(which cc)" -DCMAKE_CXX_COMPILER="$(which CC)" \
        -DCMAKE_C_FLAGS="-fopenmp" -DCMAKE_CXX_FLAGS="-fopenmp" \
        ..

  make -j 12
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

function buildNeser {
  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.neser"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"

  BUILD_TYPE="Release"

  echo "Creating Module File"
cat <<'EOF' > "${INSHIMTU_BUILD_DIR}/module.init"
  module use /sw/csg/modulefiles/applications
  module use /sw/csg/modulefiles/compilers
  module use /sw/csg/modulefiles/libs

  module use /sw/vis/ibex-gpu.modules
  module add CMake/3.12.1
  module add ParaView/5.6.0-openmpi-x86_64
EOF
  source "${INSHIMTU_BUILD_DIR}/module.init"

  cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" -DBOOST_ROOT="${BOOST_DIR}" ..

  make -j 8
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

function buildKVL {
  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.kvl"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"

  local BUILD_TYPE="RelWithDebInfo"

  echo "Creating Module File"
cat <<'EOF' > "${INSHIMTU_BUILD_DIR}/module.init"
module add kvl-applications pvserver/5.6.0-mpich-x86_64
EOF
  source "${INSHIMTU_BUILD_DIR}/module.init"

  cmake3 -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ..

  make -j 8
}


# Determine Build Recipe
case "$HOSTDOMAIN" in
*"vis.kaust.edu.sa")
  buildKVL
  ;;
*"ibex.kaust.edu.sa")
  buildIbex
  ;;
*"hpc.kaust.edu.sa")
  case "$OSVERSION" in
  "RedHat"*)
    buildNeser
    ;;
  "SUSE"*)
    buildShaheen
    ;;
  *)
    echo "Unknown build environment."
    exit 1
    ;;
  esac
  ;;
*)
  case "$OSVERSION" in
  "RedHat"*)
    buildNeser
    ;;
  "SUSE"*)
    # host domain not available on Cray compute nodes
    buildShaheen
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


