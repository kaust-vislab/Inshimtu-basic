#!/usr/bin/sh -e
module add kvl-applications paraview/5.3.0-mpich-x86_64

export INSHIMTU_DIR
INSHIMTU_DIR="$(dirname "$(dirname "$(cd "$(dirname "$0")" && pwd)")")"

export INSHIMTU_EXEC="${INSHIMTU_DIR}/build.kvl/Inshimtu"

if [ ! -e "${INSHIMTU_EXEC}" ]; then
  echo "Expecting executable: ${INSHIMTU_EXEC}"
  exit 1
fi

AVAILABLE_HOSTS="gpgpu-00,gpgpu-01,gpgpu-02,gpgpu-03,gpgpu-04,gpgpu-05,gpgpu-06,gpgpu-07,gpgpu-08,gpgpu-09,gpgpu-10,gpgpu-11,gpgpu-12,gpgpu-13,gpgpu-14,gpgpu-15"

NODES=8
HOSTS=$(echo $NODES $AVAILABLE_HOSTS | awk '{ split($2,a,","); hs=a[1]; for (x=2; x<=$1; x++) hs=hs "," a[x]; print hs; }')

echo "Lanching Inshimtu from directory: ${INSHIMTU_DIR}/build.kvl"

cd "$INSHIMTU_DIR/examples"

mpirun -np $NODES -ppn 1 -hosts "$HOSTS" "${INSHIMTU_EXEC}" \
    -w "${INSHIMTU_DIR}/examples/testing/GDM" \
    -d "${INSHIMTU_DIR}/examples/testing.done" \
    -i "${INSHIMTU_DIR}/examples/testing/GDM/"wrfout_d01_2015-{10-28,10-29,10-30,10-31,11-01}* \
    -s "${INSHIMTU_DIR}/examples/pipelines/viewer_P_QICE_nodec_z2fe_22222.py" -v P,QICE

