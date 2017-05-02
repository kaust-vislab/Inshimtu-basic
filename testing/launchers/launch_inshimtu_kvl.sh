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

NODES=5
HOSTS=$(echo $NODES $AVAILABLE_HOSTS | awk '{ split($2,a,","); hs=a[1]; for (x=2; x<=$1; x++) hs=hs "," a[x]; print hs; }')

echo "Lanching Inshimtu from directory: ${INSHIMTU_DIR}/build.kvl"
#"${INSHIMTU_EXEC}" -w "${INSHIMTU_DIR}/build/testing" -d "${INSHIMTU_DIR}/build/testing.done" -s "${INSHIMTU_DIR}/testing/pipelines/gridviewer_glendon_22222.py" -i ${INSHIMTU_DIR}/build/testing/filename_*.vti

#mpiexec -np 2 -ppn 2 "${INSHIMTU_EXEC}" -h

#mpiexec -np $NODES -ppn 1 -hosts "$HOSTS" "${INSHIMTU_EXEC}" -w "${INSHIMTU_DIR}/build/testing" -d "${INSHIMTU_DIR}/build/testing.done" -s "${INSHIMTU_DIR}/testing/pipelines/gridviewer_glendon_22222.py" -i ${INSHIMTU_DIR}/build/testing/filename_*.vti

#mpiexec -np $NODES -ppn 1 -hosts "$HOSTS" "${INSHIMTU_EXEC}" -w "${INSHIMTU_DIR}/build/testing" -d "${INSHIMTU_DIR}/build/testing.done" -s "${INSHIMTU_DIR}/testing/pipelines/gridviewer_glendon_22222.py"

#mpiexec -np $NODES -ppn 1 -hosts "$HOSTS" "${INSHIMTU_EXEC}" \
#        -w "${INSHIMTU_DIR}/examples/testing/WSM3" -d "${INSHIMTU_DIR}/examples/testing.done" \
#        -i "${INSHIMTU_DIR}/examples/testing/WSM3/"WSM3_wrfout_*.nc \
#        -s "${INSHIMTU_DIR}/examples/pipelines/gridwriter_P.py" \
#        -v U,V,W,QVAPOR,P

mpiexec -np $NODES -ppn 1 -hosts "$HOSTS" "${INSHIMTU_EXEC}" \
        -w "${INSHIMTU_DIR}/examples/testing/GDM" -d "${INSHIMTU_DIR}/examples/testing.done" \
        -i "${INSHIMTU_DIR}/examples/testing/GDM/"wrfout_d01_*.nc \
        -s "${INSHIMTU_DIR}/examples/pipelines/gridwriter_ICE.py" \
        -v U,V,W,QVAPOR,QICE,P
