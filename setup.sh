#!/usr/bin/sh -e
module add kvl-applications paraview/4.4.0-mpich-x86_64

# Needed because paraview/4.4.0-mpich-x86_64 doesn't set PYTHONPATHs correctly 
module add dev-inshimtu

export INSHIMTU_DIR=$(cd `dirname $0` && pwd)


echo "Setting Inshimtu build directory: ${INSHIMTU_DIR}/build"
cd "${INSHIMTU_DIR}/build"
cmake ..

