#!/usr/bin/sh -e
module add kvl-applications paraview/5.3.0-mpich-x86_64

export INSHIMTU_DIR
INSHIMTU_DIR="$(dirname "$(dirname "$(cd "$(dirname "$0")" && pwd)")")"

export INSHIMTU_EXEC="${INSHIMTU_DIR}/build.kvl/Inshimtu"

if [ ! -e "${INSHIMTU_EXEC}" ]; then
  echo "Expecting executable: ${INSHIMTU_EXEC}"
  exit 1
fi

echo "Lanching Inshimtu from directory: ${INSHIMTU_DIR}/build.kvl"

cd "$INSHIMTU_DIR/examples"

"${INSHIMTU_EXEC}" \
    -w "${INSHIMTU_DIR}/examples/testing/GDM" \
    -d "${INSHIMTU_DIR}/examples/testing.done" \
    -i "${INSHIMTU_DIR}/examples/testing/GDM/"wrfout_d01_* \
    -s "${INSHIMTU_DIR}/examples/pipelines/viewer_QVAPOR_z2fe_22222.py" -v QVAPOR

