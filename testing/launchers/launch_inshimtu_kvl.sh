#!/usr/bin/sh -e
module add kvl-applications paraview/5.3.0-mpich-x86_64

export INSHIMTU_DIR
INSHIMTU_DIR="$(dirname "$(dirname "$(cd "$(dirname "$0")" && pwd)")")"

export INSHIMTU_EXEC="${INSHIMTU_DIR}/build.kvl/Inshimtu"

if [ ! -x "${INSHIMTU_EXEC}" ]; then
  echo "Expecting executable: ${INSHIMTU_EXEC}"
  exit 1
fi

export DEFAULT_DATA_DIR="${INSHIMTU_DIR}/testing/data"


AVAILABLE_HOSTS_GPGPU="gpgpu-00,gpgpu-01,gpgpu-02,gpgpu-03,gpgpu-04,gpgpu-05,gpgpu-06,gpgpu-07,gpgpu-08,gpgpu-09,gpgpu-10,gpgpu-11,gpgpu-12,gpgpu-13,gpgpu-14,gpgpu-15"
MAX_AVAILABLE_HOSTS_GPGPU=16
AVAILABLE_HOSTS_CORNEA="pc102,pc103,pc104,pc105,pc106,pc107,pc108,pc109,pc110,pc111,pc112,pc113"
MAX_AVAILABLE_HOSTS_CORNEA=12
AVAILABLE_HOSTS_CUBES="vis-cubes-0,vis-cubes-1,vis-cubes-2,vis-cubes-3,vis-cubes-4,vis-cubes-5,vis-cubes-6"
MAX_AVAILABLE_HOSTS_CUBES=7
AVAILABLE_HOSTS_Z5="z5-0,z5-1,z5-2"
MAX_AVAILABLE_HOSTS_Z5=3
AVAILABLE_HOSTS_Z2="z2-0,z2-1,z2-2,z2-3,z2-4"
MAX_AVAILABLE_HOSTS_Z2=5
AVAILABLE_HOSTS_Z1="z1-fe"
MAX_AVAILABLE_HOSTS_Z1=1
AVAILABLE_HOSTS_LMEM="lmem-00"
MAX_AVAILABLE_HOSTS_LMEM=1


export HOST_DOMAINNAME
HOST_DOMAINNAME="$(hostname -d)"
HOST_DOMAINNAME=${HOST_DOMAINNAME=//.vis.kaust.edu.sa/}

export HOST_NAME
HOST_NAME="$(hostname -s)"


# Setup hosts
case "$HOST_DOMAINNAME" in
  "gpgpu"*)
    echo "Setup GPGPU hosts"
    AVAILABLE_HOSTS=$AVAILABLE_HOSTS_GPGPU
    MAX_AVAILABLE_HOSTS=$MAX_AVAILABLE_HOSTS_GPGPU
    ;;
  "cornea")
    echo "Setup CORNEA hosts"
    AVAILABLE_HOSTS=$AVAILABLE_HOSTS_CORNEA
    MAX_AVAILABLE_HOSTS=$MAX_AVAILABLE_HOSTS_CORNEA
    ;;
  "cubes")
    echo "Setup CUBES hosts"
    AVAILABLE_HOSTS=$AVAILABLE_HOSTS_CUBES
    MAX_AVAILABLE_HOSTS=$MAX_AVAILABLE_HOSTS_CUBES
    ;;
  "z5")
    echo "Setup Z5 hosts"
    AVAILABLE_HOSTS=$AVAILABLE_HOSTS_Z5
    MAX_AVAILABLE_HOSTS=$MAX_AVAILABLE_HOSTS_Z5
    ;;
  "z2")
    echo "Setup Z2 hosts"
    AVAILABLE_HOSTS=$AVAILABLE_HOSTS_Z2
    MAX_AVAILABLE_HOSTS=$MAX_AVAILABLE_HOSTS_Z2
    ;;
  "z1")
    echo "Setup Z1 hosts"
    AVAILABLE_HOSTS=$AVAILABLE_HOSTS_Z1
    MAX_AVAILABLE_HOSTS=$MAX_AVAILABLE_HOSTS_Z1
    ;;
  "common")
    case "$HOST_NAME" in
      "lmem"*)
        echo "Setup LMEM hosts"
        AVAILABLE_HOSTS=$AVAILABLE_HOSTS_LMEM
        MAX_AVAILABLE_HOSTS=$MAX_AVAILABLE_HOSTS_LMEM
        ;;
      *)
        echo "Setup generic COMMON host: $HOST_NAME"
        AVAILABLE_HOSTS=$HOST_NAME
        MAX_AVAILABLE_HOSTS=1
    esac
    ;;
  "desktop")
    echo "Setup DESKTOP host: $HOST_NAME"
    AVAILABLE_HOSTS=$HOST_NAME
    MAX_AVAILABLE_HOSTS=1
    ;;
  *)
    echo "Setup generic host: $HOST_NAME"
    AVAILABLE_HOSTS=$HOST_NAME
    MAX_AVAILABLE_HOSTS=1
    ;;
esac



#
# Usage function
#
usage() {
    echo "Usage: $0 [-D data-directory] [-f file-watch-regex] [-i initial-files-glob] [-N total-nodes-range] [-n inporter-node-ranges] [-s script] [-v variables-list] [-S scenarios] [-h help]" 1>&2;
}

#
# Use getopts to get arguments
#
while getopts ":D:d:f:i:N:n:s:v:S:h" opt; do
  case $opt in
    D)
      DATA_DIR=$OPTARG
      ;;
    d)
      DONE_FILE=$OPTARG
      ;;
    f)
      FILES_WATCH_REGEX=$OPTARG
      ;;
    i)
      INITIAL_FILES_GLOB=$OPTARG
      ;;
    N)
      NODE_RANGE=$OPTARG
      START_NODE=${NODE_RANGE%-*}
      END_NODE=${NODE_RANGE#*-}

      NODE=$((START_NODE))
      if [ ! $NODE = "$START_NODE" ]; then
        echo "Error in option -N: <start-node>-<end-node> must be integers."
        usage
        exit 1
      fi
      NODE=$((END_NODE))
      if [ ! $NODE = "$END_NODE" ]; then
        echo "Error in option -N: <start-node>-<end-node> must be integers."
        usage
        exit 1
      fi
      ;;
    n)
      INPORTER_NODES=$OPTARG
      ;;
    s)
      SCRIPT_FILE=$OPTARG
      ;;
    v)
      VARIABLES_LIST=$OPTARG
      ;;
    S)
      case "$OPTARG" in
        "GDM")
          echo "Setup GDM configuration"
          DATA_DIR="/lustre/project/k1029/hari/chapalla/GDM"
          INITIAL_FILES_GLOB='wrfout_d01_*'
          SCRIPT_FILE="${INSHIMTU_DIR}/testing/pipelines/gridviewer.py" \
          VARIABLES_LIST="U,V,W,QVAPOR"
          INPORTER_NODES=0
        ;;
        *)
      esac
      ;;
    h)
      usage
      echo "Scenarios: GDM" 1>&2;
      exit 1
      ;;
    [?])
      echo "Invalid option: -$OPTARG" 1>&2;
      usage
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." 1>&2;
      usage
      exit 1
      ;;
  esac
done
shift $((OPTIND-1))

# Validate
if [ -z "${SCRIPT_FILE}" ] || [ -z "${VARIABLES_LIST}" ] ; then
  echo "Missing required options: [-s script] [-v variables-list]" 1>&2;
  usage
  exit 1
fi

# Handle Nodes
START_NODE=${START_NODE:-1}
END_NODE=${END_NODE:-$(( MAX_AVAILABLE_HOSTS ))}
MAX_NODES=$(( 1 + END_NODE - START_NODE ))
NODES=$MAX_AVAILABLE_HOSTS
if [ "$NODES" -gt "$MAX_NODES" ]; then
  NODES=$MAX_NODES
fi
END_NODE=$(( START_NODE + NODES - 1 ))

HOSTS=$(echo "$START_NODE" "$END_NODE" "$AVAILABLE_HOSTS" | \
        awk '{ split($3,a,","); hs=a[$1]; for (x=$1+1; x<=$2; x++) hs=hs "," a[x]; print hs; }')


# Handle Input Data and Files
DATA_DIR=${DATA_DIR:-$DEFAULT_DATA_DIR}

if [ -z "$DONE_FILE" ] && [ -z "$INITIAL_FILES_GLOB" ]; then
  DONE_FILE=${DONE_FILE:-"${DATA_DIR}.done"}
  touch "$DONE_FILE"
fi


# Launch

echo "Lanching Inshimtu: ${INSHIMTU_EXEC}"


if [ -n "$FILES_WATCH_REGEX" ]; then
  OptWATCHREGEX="-f '$FILES_WATCH_REGEX'"
fi

if [ -n "$DATA_DIR" ] && [ -n "$DONE_FILE" ]; then
  OptWATCHING="-w "${DATA_DIR}" -d "$DONE_FILE" $OptWATCHREGEX"
fi

if [ -n "$INITIAL_FILES_GLOB" ]; then
  OptINFILES="-i "${DATA_DIR}/"${INITIAL_FILES_GLOB}"
fi

if [ -n "$INPORTER_NODES" ]; then
  OptINNODES="-n "$INPORTER_NODES""
fi

if [ -n "$SCRIPT_FILE" ] && [ -n "$VARIABLES_LIST" ]; then
  OptCATALYST="-s "${SCRIPT_FILE}" -v "$VARIABLES_LIST" $OptINNODES"
fi


mpiexec -np $NODES -ppn 1 -hosts "$HOSTS" \
        "${INSHIMTU_EXEC}" $OptWATCHING $OptINFILES $OptCATALYST


