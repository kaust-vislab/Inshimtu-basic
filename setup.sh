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
    LIB_EXT="a"
    BUILD_TYPE="Release"
  fi


  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.shaheen"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"


  # Enable Cray profiling tools
  module unload darshan

  module swap PrgEnv-cray PrgEnv-gnu
  module add cdt/17.12

  module add cmake/3.10.2

  echo "Creating Module File"
cat <<'EOF' > "${INSHIMTU_BUILD_DIR}/module.init"
  module use /sw/vis/xc40.modules
  module add ParaView/5.4.1-CrayGNU-2017.12.KSL-server-mesa
  module add boost/1.66-gcc-7.2.0
  #module add cray-netcdf-hdf5parallel/4.4.1.1.6
  module add cray-parallel-netcdf/1.8.1.3
  module add cray-hdf5-parallel/1.10.1.1
  # TODO: Put fix in ParaView module
  #   Fix for issue loading correct version of cray mpi
  #   export LD_LIBRARY_PATH="$CRAY_LD_LIBRARY_PATH":$LD_LIBRARY_PATH
  export LD_LIBRARY_PATH="${CRAY_MPICH2_DIR}/lib":$LD_LIBRARY_PATH
EOF
  source "${INSHIMTU_BUILD_DIR}/module.init"

  if [ "$INSHIMTU_PROFILING" = true ] ; then
    # Enable Cray profiling tools
    module load perftools-base perftools
    # Allinea support
    # TODO: restore allinea support when available again
    #module load allinea-reports/7.0 allinea-forge/7.0
  fi


  if [ "$INSHIMTU_PROFILING" = true ] ; then
    # Allinea support
    # TODO: restore allinea support when available again
    module av allinea-reports
    module av allinea-forge
    #make-profiler-libraries
  fi

  # TODO: cmake should be able to find correct MPI: find_package(MPI REQUIRED)
  cmake -DCMAKE_SYSTEM_NAME=CrayLinuxEnvironment -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DMPI_C_INCLUDE_PATH="${MPICH_DIR}/include" -DMPI_CXX_INCLUDE_PATH="${MPICH_DIR}/include" \
        -DMPI_C_LIBRARIES="${MPICH_DIR}/lib/libmpich.${LIB_EXT}" \
        -DMPI_CXX_LIBRARIES="${MPICH_DIR}/lib/libmpichcxx.${LIB_EXT}" \
        -DCMAKE_C_COMPILER="$(which cc)" -DCMAKE_CXX_COMPILER="$(which CC)" \
        ..

  make -j 12


  if [ "$INSHIMTU_PROFILING" = true ] ; then
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
  module add cmake/3.9.4/gnu-6.4.0
  module use /sw/vis/ibex-gpu.modules
  module add ParaView/5.4.1-openmpi-x86_64
EOF
  source "${INSHIMTU_BUILD_DIR}/module.init"

  cmake -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" ..

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
module add kvl-applications pvserver/5.4.1-mpich-x86_64
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
*"dragon.kaust.edu.sa")
  ;&
*"ibex.kaust.edu.sa")
  buildIbex
  ;;
*"hpc.kaust.edu.sa")
  buildShaheen
  ;;
*)
  case "$OSVERSION" in
  "SUSE LINUX"*)
    ;&
  "SUSE 12"*)
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


