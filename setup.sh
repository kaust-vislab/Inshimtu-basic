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


  INSHIMTU_PROFILING=true
  if [ "$INSHIMTU_PROFILING" = true ] ; then
    LIB_EXT="so"
  else
    LIB_EXT="a"
  fi


  # Enable Cray profiling tools
  module unload darshan

  module swap PrgEnv-cray PrgEnv-gnu
  module add cdt/16.07

  module add cmake/3.6.2

  module use /lustre/project/k1033/software/staging.2017.04/easybuild/modules/all
  module add Boost/1.61.0-CrayGNU-2016.07.KSL
  module add ParaView/5.3.0-CrayGNU-2016.07.KSL-server-mesa
  module add cray-hdf5-parallel/1.10.0.1

  if [ "$INSHIMTU_PROFILING" = true ] ; then
    # Enable Cray profiling tools
    module load perftools-base perftools
    # Allinea support
    module load allinea-reports/7.0 allinea-forge/7.0
  fi


  echo "Setting Inshimtu build directory: ${INSHIMTU_DIR}/build.shaheen"
  mkdir "${INSHIMTU_DIR}/build.shaheen"
  cd "${INSHIMTU_DIR}/build.shaheen"


  if [ "$INSHIMTU_PROFILING" = true ] ; then
    # Allinea support
    make-profiler-libraries
  fi


  cmake -DCMAKE_SYSTEM_NAME=CrayLinuxEnvironment -DMPI_C_INCLUDE_PATH="${MPICH_DIR}/include" -DMPI_CXX_INCLUDE_PATH="${MPICH_DIR}/include" -DMPI_C_LIBRARIES="${MPICH_DIR}/lib/libmpich.${LIB_EXT}" -DMPI_CXX_LIBRARIES="${MPICH_DIR}/lib/libmpichcxx.${LIB_EXT}" -DCMAKE_C_COMPILER="$(which cc)" -DCMAKE_CXX_COMPILER="$(which CC)" -DBOOST_ROOT=$EBROOTBOOST ..

  make -j 12


  if [ "$INSHIMTU_PROFILING" = true ] ; then
    pat_build Inshimtu
      # TODO:
      # To instrument a program, add these compiler options:
      #   compilation for use with MAP - not required for Performance Reports:
      #     -g (or '-G2' for native Cray Fortran) (and -O3 etc.)
      #   linking (both MAP and Performance Reports):
      #     -dynamic -L/lustre/project/k1033/Development/Inshimtu/build.shaheen -lmap-sampler-pmpi -lmap-sampler \
      #              -Wl,--eh-frame-hdr -Wl,-rpath=/lustre/project/k1033/Development/Inshimtu/build.shaheen
      # Note: These libraries must be on the same NFS/Lustre/GPFS filesystem as your program.
      # Before running your program (interactively or from a queue), set LD_LIBRARY_PATH:
      #    export LD_LIBRARY_PATH=/lustre/project/k1033/Development/Inshimtu/build.shaheen:$LD_LIBRARY_PATH
      #    map --profile Inshimtu ... 
      # or add  when linking your program: 
      #    -Wl,-rpath=/lustre/project/k1033/Development/Inshimtu/build.shaheen
  fi

  ;;
*)
  echo "Unknown build environment"
  ;;
esac


