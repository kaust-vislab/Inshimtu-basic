#!/bin/sh -e

export INSHIMTU_DIR=$(cd `dirname $0` && pwd)

OSVERSION=$(echo `lsb_release -sir` | awk -F '.' '{ print $1 }')
HOSTDOMAIN=$(hostname -d)


function buildShaheen {
  # NOTE: Build on compute node:
  # salloc --partition=debug
  # srun -u --pty bash -i
  # cd /lustre/sw/vis/development/Inshimtu
  # ./setup.sh


  INSHIMTU_PROFILING=true
  if [ "$INSHIMTU_PROFILING" = true ] ; then
    LIB_EXT="so"
    BUILD_TYPE="RelWithDebInfo"
  else
    LIB_EXT="a"
    BUILD_TYPE="Release"
  fi


  # Enable Cray profiling tools
  module unload darshan

  module swap PrgEnv-cray PrgEnv-gnu
  module add cdt/16.07

  module add cmake/3.6.2

  module use /lustre/sw/vis/modulefiles
  module add Boost/1.61.0-CrayGNU-2016.07.KSL
  module add ParaView/5.4.1-CrayGNU-2016.07.KSL-server-mesa
  module add cray-hdf5-parallel/1.10.0.1

  if [ "$INSHIMTU_PROFILING" = true ] ; then
    # Enable Cray profiling tools
    module load perftools-base perftools
    # Allinea support
    module load allinea-reports/7.0 allinea-forge/7.0
  fi

  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.shaheen"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"


  if [ "$INSHIMTU_PROFILING" = true ] ; then
    # Allinea support
    make-profiler-libraries
  fi


  cmake -DCMAKE_SYSTEM_NAME=CrayLinuxEnvironment -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DMPI_C_INCLUDE_PATH="${MPICH_DIR}/include" -DMPI_CXX_INCLUDE_PATH="${MPICH_DIR}/include" \
        -DMPI_C_LIBRARIES="${MPICH_DIR}/lib/libmpich.${LIB_EXT}" \
        -DMPI_CXX_LIBRARIES="${MPICH_DIR}/lib/libmpichcxx.${LIB_EXT}" \
        -DCMAKE_C_COMPILER="$(which cc)" -DCMAKE_CXX_COMPILER="$(which CC)" \
        -DBOOST_ROOT=$EBROOTBOOST \
        ..

  make -j 12


  if [ "$INSHIMTU_PROFILING" = true ] ; then
    pat_build Inshimtu
  fi
}

function buildCluster {
  module use /sw/vis/cluster-gpu/easybuild/modules/all
  module add CMake/3.5.2 ParaView/5.4.1-openmpi-x86_64

  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.cluster"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"

  cmake ..

  make -j 8
}

function buildKVL {
  module add kvl-applications pvserver/5.4.1-mpich-x86_64

  INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.kvl"
  echo "Setting Inshimtu build directory: ${INSHIMTU_BUILD_DIR}"
  mkdir "${INSHIMTU_BUILD_DIR}"
  cd "${INSHIMTU_BUILD_DIR}"

  cmake3 ..

  make -j 8
}


# Determine Build Recipe
case "$HOSTDOMAIN" in
*"vis.kaust.edu.sa")
  buildKVL
  ;;
*"dragon.kaust.edu.sa")
  buildCluster
  ;;
*"hpc.kaust.edu.sa")
  buildShaheen
  ;;
*)
  case "$OSVERSION" in
  "SUSE LINUX"*)
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


