#!/usr/bin/sh -e
module add kvl-applications paraview

export INSHIMTU_DIR=$(cd `dirname $0` && pwd)


echo "Setting Inshimtu build directory: ${INSHIMTU_DIR}/build"
cd "${INSHIMTU_DIR}/build"
cmake ..

