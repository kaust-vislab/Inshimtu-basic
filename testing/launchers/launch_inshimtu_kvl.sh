#!/usr/bin/sh -e
export INSHIMTU_DIR
INSHIMTU_DIR="$(dirname "$(dirname "$(cd "$(dirname "$0")" && pwd)")")"

export INSHIMTU_BUILD_DIR="${INSHIMTU_DIR}/build.kvl"
export INSHIMTU_EXEC="${INSHIMTU_BUILD_DIR}/Inshimtu"
export INSHIMTU_MODULES="${INSHIMTU_BUILD_DIR}/module.init"

if [ ! -x "${INSHIMTU_EXEC}" ]; then
  echo "Expecting executable: ${INSHIMTU_EXEC}"
  exit 1
fi

if [ ! -e "${INSHIMTU_MODULES}" ]; then
  echo "Expecting modules: ${INSHIMTU_MODULES}"
  exit 1
fi
source "${INSHIMTU_MODULES}"

export DEFAULT_DATA_DIR="${INSHIMTU_DIR}/testing/data"


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
  echo "Scenarios: GDM, GDMcmdline, CycloneExtract, TestReadyFiles, TestMultinodeWrite" 1>&2;
}


#
# Helper functions
#
createHelperScript() {
HELPER_SCRIPT=$1
HELPER_DIR="$(dirname "$HELPER_SCRIPT")"
if [ ! -d "$HELPER_DIR" ] ; then
  echo "Invalid HELPER SCRIPT: ${HELPER_SCRIPT}"
  exit 1
fi

echo "Creating HELPER: ${HELPER_SCRIPT}"
cat <<'EOF' > "${HELPER_SCRIPT}"
#!/usr/bin/sh -e
echo "Running HELPER: ${HELPER_SCRIPT}"
echo "Initial Delay... 40s"
sleep 40
EOF
}


#
# Scenarios
#

scenarioGDM() {
  #GDM -> /var/remote/projects/kaust/earthenvironscience/hari/cyclone/GDM/
  echo "Setup GDM configuration"
  WORK_DIR="${INSHIMTU_DIR}/examples/test-GDM"
  CONFIG_FILE="${WORK_DIR}/config.json"
  DATA_DIR="${WORK_DIR}/data"
  mkdir -p "${WORK_DIR}"
  cd "${WORK_DIR}"
  ln -fns "${INSHIMTU_DIR}/examples/data/GDM" "${DATA_DIR}" 
  ln -fs "${INSHIMTU_DIR}/testing/configs/gdm_relpath.json" "${CONFIG_FILE}" 
  ln -fs "${INSHIMTU_DIR}/testing/pipelines" "${WORK_DIR}" 
}

scenarioGDMcmdline() {
  #GDM -> /var/remote/projects/kaust/earthenvironscience/hari/cyclone/GDM/
  echo "Setup GDM-command-line configuration"
  DATA_DIR="${INSHIMTU_DIR}/examples/data/GDM"
  INITIAL_FILES_GLOB='wrfout_d01_*'
  SCRIPT_FILE="${INSHIMTU_DIR}/testing/pipelines/gridviewer_gdm_UVWQVAPOR.py" \
  VARIABLES_LIST="U,V,W,QVAPOR"
  INPORTER_NODES=0
}

scenarioCycloneExtract() {
  #GDMncdata -> /var/remote/projects/kaust/earthenvironscience/hari/tom_playground/inshimtu/data/
  echo "Setup CycloneExtract configuration"
  WORK_DIR="${INSHIMTU_DIR}/examples/test-cycloneextract"
  CONFIG_FILE="${WORK_DIR}/config.json"
  DATA_DIR="${WORK_DIR}/data"
  mkdir -p "${WORK_DIR}"
  cd "${WORK_DIR}"
  ln -fns "${INSHIMTU_DIR}/examples/data/GDMncdata" "${DATA_DIR}" 
  ln -fs "${INSHIMTU_DIR}/testing/pipelines/cyclone_extract/cyclone_configs.json" "${CONFIG_FILE}" 
  ln -fs "${INSHIMTU_DIR}/testing/pipelines" "${WORK_DIR}"
}

scenarioTestReadyFiles() {
  #GDM -> /var/remote/projects/kaust/earthenvironscience/hari/cyclone/GDM/
  echo "Setup TestReadyFile configuration"
  WORK_DIR="${INSHIMTU_DIR}/examples/test-outready"
  CONFIG_FILE="${WORK_DIR}/config.json"
  DATA_DIR="${WORK_DIR}/data"
  COHELPER_SCRIPT_SH="${WORK_DIR}/doDelayedReady.sh"
  mkdir -p "${WORK_DIR}"

  mkdir -p "${DATA_DIR}"
  rm -f "${DATA_DIR}/"wrfoutReady_*
  cd "${DATA_DIR}"
  for dfile in "${INSHIMTU_DIR}/examples/data/GDM"/wrfout_* ; do
    ln -fs "$dfile" "${DATA_DIR}"
  done

  cd "${WORK_DIR}"
  ln -fs "${INSHIMTU_DIR}/testing/configs/gdm_outready.json" "${CONFIG_FILE}" 
  ln -fs "${INSHIMTU_DIR}/testing/pipelines" "${WORK_DIR}" 
  touch "${WORK_DIR}/data.done"

  createHelperScript "$COHELPER_SCRIPT_SH"
  for dfile in "${DATA_DIR}"/wrfout_* ; do
    dirpath="$(dirname "${dfile}")"
    fname="$(basename "${dfile}")"
    rfile="${dirpath}/${fname/wrfout_/wrfoutReady_}"
    echo "touch $rfile" >> "$COHELPER_SCRIPT_SH"
    echo "echo "Helper waiting... 20s"" >> "$COHELPER_SCRIPT_SH"
    echo "sleep 20" >> "$COHELPER_SCRIPT_SH"
  done
  chmod +x "$COHELPER_SCRIPT_SH"
}

scenarioTestMultinodeWrite() {
  #testing = {filename_*_0.vti}
  #<VTKFile type="ImageData" version="1.0" byte_order="LittleEndian" header_type="UInt64">
  #<ImageData WholeExtent="0 1024 0 128 0 64" Origin="0 0 0" Spacing="1 1.1 1.3">
  #  <Piece Extent="0 1024 0 128 0 64"                                                 >
  #    <PointData>
  #      <DataArray type="Float64" Name="velocity" NumberOfComponents="3" .../>
  #    </PointData>
  #    <CellData>
  #      <DataArray type="Float32" Name="pressure" .../>
  #    </CellData>
  #  </Piece>
  #</ImageData>
  echo "Setup TestMultinodeWrite configuration"
  WORK_DIR="${INSHIMTU_DIR}/examples/test-multinodewrite"
  CONFIG_FILE="${WORK_DIR}/config.json"
  DATA_DIR="${WORK_DIR}/data"
  COHELPER_SCRIPT_SH="${WORK_DIR}/doDelayedTouch.sh"
  mkdir -p "${WORK_DIR}"

  mkdir -p "${DATA_DIR}"
  rsync "${INSHIMTU_DIR}/examples/data/testing"/filename_*.vti "${DATA_DIR}"

  cd "${WORK_DIR}"
  ln -fs "${INSHIMTU_DIR}/testing/configs/vti_notified.json" "${CONFIG_FILE}" 
  ln -fs "${INSHIMTU_DIR}/testing/pipelines" "${WORK_DIR}" 
  touch "${WORK_DIR}/data.done"

  createHelperScript "$COHELPER_SCRIPT_SH"
  for dfile in "${DATA_DIR}"/filename_*.vti ; do
    echo "touch $dfile" >> "$COHELPER_SCRIPT_SH"
    echo "echo "Helper waiting... 20s"" >> "$COHELPER_SCRIPT_SH"
    echo "sleep 20" >> "$COHELPER_SCRIPT_SH"
  done
  chmod +x "$COHELPER_SCRIPT_SH"

  AVAILABLE_HOSTS="$HOST_NAME,$AVAILABLE_HOSTS_LMEM"
  MAX_AVAILABLE_HOSTS=2

  echo "Run ${COHELPER_SCRIPT_SH} on nodes: ${AVAILABLE_HOSTS}" 1>&2;
  echo "  ${COHELPER_SCRIPT_SH} &" 1>&2;
  echo "  ssh $AVAILABLE_HOSTS_LMEM ${COHELPER_SCRIPT_SH}" 1>&2;
  unset COHELPER_SCRIPT_SH
}


#
# Use getopts to get arguments
#
while getopts ":D:d:f:i:N:n:c:s:v:S:h" opt; do
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
    c)
      CONFIG_FILE=$OPTARG
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
          scenarioGDM
        ;;
        "GDMcmdline")
          scenarioGDMcmdline
        ;;
        "CycloneExtract")
          scenarioCycloneExtract
        ;;
        "TestReadyFiles")
          scenarioTestReadyFiles
        ;;
        "TestMultinodeWrite")
          scenarioTestMultinodeWrite
        ;;
        *)
          echo "Unknown scenario: $OPTARG" 1>&2;
          usage
          exit 1
        ;;
      esac
      ;;
    h)
      usage
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
if [ -z "${CONFIG_FILE}" ] && ([ -z "${SCRIPT_FILE}" ] || [ -z "${VARIABLES_LIST}" ]) ; then
  echo "Missing required options: [-c config]  [-s script] [-v variables-list]" 1>&2;
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

if [ -z "$CONFIG_FILE" ] && [ -z "$DONE_FILE" ] && [ -z "$INITIAL_FILES_GLOB" ]; then
  DONE_FILE=${DONE_FILE:-"${DATA_DIR}.done"}
  touch "$DONE_FILE"
fi


# Handle CoProcessing Helper Scripts
if [ -x "${COHELPER_SCRIPT_SH}" ] ; then
  "${COHELPER_SCRIPT_SH}" &
  trap 'kill $(jobs -p)' EXIT
elif [ -n "${COHELPER_SCRIPT_SH}" ] ; then
  echo "HELPER SCRIPT is not executable: ${COHELPER_SCRIPT_SH}"
  exit 1
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

if [ -n "$CONFIG_FILE" ]; then
  OptCONFIG="-c "$CONFIG_FILE""
fi


mpiexec -np $NODES -ppn 1 -hosts "$HOSTS" \
        "${INSHIMTU_EXEC}" $OptWATCHING $OptINFILES $OptCATALYST $OptCONFIG


