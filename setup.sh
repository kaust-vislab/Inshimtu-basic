#!/bin/sh -e

export INSHIMTU_DIR=$(cd `dirname $0` && pwd)

OSVERSION=$(echo `lsb_release -sir` | awk -F '.' '{ print $1 }')
case "$OSVERSION" in
"CentOS"*)
  module add kvl-applications paraview/4.4.0-mpich-x86_64

  echo "Setting Inshimtu build directory: ${INSHIMTU_DIR}/build.kvl"
  mkdir "${INSHIMTU_DIR}/build.kvl"
  cd "${INSHIMTU_DIR}/build.kvl"

  cmake ..

  make -j 8

  ;;
"SUSE LINUX"*)

  # TODO: doesn't build against paraview/4.4.0; possibly not built with GCC?; has link error with std:: libs
  #module add paraview/4.4.0 

  module sw PrgEnv-cray PrgEnv-gnu

  # Enable profiling tools
  module unload darshan
  module load perftools-base/6.3.2 perftools/6.3.2

  module add cmake/3.1.3
  module add cray-hdf5

  # Note: Issue is that loading boost changes gcc from gcc/5.1.0 to gcc/4.9.3
  module add boost

  # Implied by paraview
  # > module sw PrgEnv-cray PrgEnv-gnu
  module sw PrgEnv-gnu PrgEnv-cray
  module add paraview/4.3.1
  
  echo "Setting Inshimtu build directory: ${INSHIMTU_DIR}/build.shaheen"
  mkdir "${INSHIMTU_DIR}/build.shaheen"
  cd "${INSHIMTU_DIR}/build.shaheen"

  cmake -DMPI_C_INCLUDE_PATH="${MPICH_DIR}/include" -DMPI_CXX_INCLUDE_PATH="${MPICH_DIR}/include" -DMPI_C_LIBRARIES="${MPICH_DIR}/lib" -DMPI_CXX_LIBRARIES="${MPICH_DIR}/lib" -DCMAKE_C_COMPILER="$(which gcc)" -DCMAKE_CXX_COMPILER="$(which g++)" ..

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


