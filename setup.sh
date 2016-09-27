#!/bin/sh -e

export INSHIMTU_DIR=$(cd `dirname $0` && pwd)

OSVERSION=$(echo `lsb_release -sir` | awk -F '.' '{ print $1 }')

case "$OSVERSION" in
"CentOS"*)
  module add kvl-applications paraview/5.1.2-mpich-x86_64

  echo "Setting Inshimtu build directory: ${INSHIMTU_DIR}/build.kvl"
  mkdir "${INSHIMTU_DIR}/build.kvl"
  cd "${INSHIMTU_DIR}/build.kvl"

  cmake3 ..

  make -j 8

  ;;
"SUSE LINUX"*)

  # NOTE: Build on compute node:
  # salloc
  # srun bash -i
  # cd /lustre/project/k1033/Development/Inshimtu
  # ./setup.sh

  # Enable profiling tools
  #module unload darshan
  #module load perftools-base/6.3.2 perftools/6.3.2

  module use /lustre/project/k1033/software/easybuild/modules/all
  module add CMake
  module add Boost
  module add ParaView/5.1.2-CrayGNU-2016.07.KSL-server-mesa
  module add cray-hdf5-parallel
  
  echo "Setting Inshimtu build directory: ${INSHIMTU_DIR}/build.shaheen"
  mkdir "${INSHIMTU_DIR}/build.shaheen"
  cd "${INSHIMTU_DIR}/build.shaheen"

  cmake -DMPI_C_INCLUDE_PATH="${MPICH_DIR}/include" -DMPI_CXX_INCLUDE_PATH="${MPICH_DIR}/include" -DMPI_C_LIBRARIES="${MPICH_DIR}/lib" -DMPI_CXX_LIBRARIES="${MPICH_DIR}/lib" -DCMAKE_C_COMPILER="$(which gcc)" -DCMAKE_CXX_COMPILER="$(which g++)" -DBOOST_ROOT=$EBROOTBOOST ..

  make -j 8

  # NOTE: Profiling Disabled
  # TODO: Fix ERROR: Missing required ELF section '.note.link' from the program 'Inshimtu'. Load the correct 'perftools' module and rebuild the program.
  # NOTE: This error can indicate that perftools doesn't like the directory name (e.g., has a 'tmp' in it).
  #pat_build -S Inshimtu

  ;;
*)
  echo "Unknown build environment"
  ;;
esac


