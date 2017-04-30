#!/bin/sh -e

export INSHIMTU_DIR=$(cd `dirname $0` && pwd)

OSVERSION=$(echo `lsb_release -sir` | awk -F '.' '{ print $1 }')

case "$OSVERSION" in
"CentOS"*)
  module add kvl-applications paraview/5.3.0-mpich-x86_64

  echo "Setting Inshimtu build directory: ${INSHIMTU_DIR}/build.kvl"
  mkdir "${INSHIMTU_DIR}/build.kvl"
  cd "${INSHIMTU_DIR}/build.kvl"

  # TODO: Fix CMake Error at CMakeLists.txt:45 (add_executable):
  #   Target "Inshimtu" links to target "Qt5::X11Extras" but the target was not found.  
  #   Perhaps a find_package() call is missing for an IMPORTED target, or an ALIAS target is missing?
  # Fix: sed -i 's\-lQt5::X11Extras\\g' CMakeFiles/Inshimtu.dir/link.txt
  # Fix: find_package(...???...)


  cmake3 ..

  make -j 8

  ;;
"SUSE LINUX"*)

  # NOTE: Build on compute node:
  # salloc --partition=debug
  # srun bash -i
  # cd /lustre/project/k1033/Development/Inshimtu
  # ./setup.sh

  # Enable Cray profiling tools
  module unload darshan

  module swap PrgEnv-cray PrgEnv-gnu
  module add cdt/16.07
  module add cmake/3.6.2

  module use /lustre/project/k1033/software/staging.2017.04/easybuild/modules/all
  module add Boost/1.61.0-CrayGNU-2016.07.KSL
  module add ParaView/5.3.0-CrayGNU-2016.07.KSL-server-mesa
  module add cray-hdf5-parallel

  # Enable Cray profiling tools
  module load perftools-base perftools
  # Allinea support
  module load allinea-reports/7.0 allinea-forge/7.0
  

  echo "Setting Inshimtu build directory: ${INSHIMTU_DIR}/build.shaheen"
  mkdir "${INSHIMTU_DIR}/build.shaheen"
  cd "${INSHIMTU_DIR}/build.shaheen"

  # Allinea support
  make-profiler-libraries

  cmake -DMPI_C_INCLUDE_PATH="${MPICH_DIR}/include" -DMPI_CXX_INCLUDE_PATH="${MPICH_DIR}/include" -DMPI_C_LIBRARIES="${MPICH_DIR}/lib" -DMPI_CXX_LIBRARIES="${MPICH_DIR}/lib" -DCMAKE_C_COMPILER="$(which cc)" -DCMAKE_CXX_COMPILER="$(which CC)" -DBOOST_ROOT=$EBROOTBOOST ..

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


