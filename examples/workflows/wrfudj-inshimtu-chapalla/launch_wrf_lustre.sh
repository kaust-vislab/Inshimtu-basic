#!/bin/bash

# Note: if any step fails, cancel everything
#set -e

## FUNCTIONS

function checkFile () {
  local name="$1"
  local value="${!name}"
  if [ -z "$name" ] ; then  
    echo "Invalid var name argument to checkFile"
    exit 1
  fi
  if [ -z "$value" ] || [ ! -f "$value" ] ; then  
    echo "Missing file ${name}: ${value}"
    exit 1
  fi
}

function checkDirectory () {
  local name="$1"
  local value="${!name}"
  if [ -z "$name" ] ; then  
    echo "Invalid var name argument to checkDirectory"
    exit 1
  fi
  if [ -z "$value" ] || [ ! -d "$value" ] ; then  
    echo "Missing directory ${name}: ${value}"
    exit 1
  fi
}


## PATHS

PROJECT_DIR="$(dirname "$(realpath -s "$0")")"

WORKFLOW_DIR="$PROJECT_DIR/workflow"
WORKFLOW_CONFIG_NAME="workflow.configs"

BATCH_SCRIPT="${WORKFLOW_DIR}/launch-workflow.sbat"
WORKFLOW_SCRIPT="${WORKFLOW_DIR}/run_workflow.sh"
WORKFLOW_CONFIG="${PROJECT_DIR}/${WORKFLOW_CONFIG_NAME}"

checkDirectory PROJECT_DIR
checkDirectory WORKFLOW_DIR
checkFile BATCH_SCRIPT
checkFile WORKFLOW_SCRIPT
checkFile WORKFLOW_CONFIG


## LAUNCH

jid1=$(sbatch \
        "${BATCH_SCRIPT}" "${WORKFLOW_SCRIPT}" "${WORKFLOW_CONFIG}" \
      )
jid1=${jid1//[!0-9]/}

#source "${BATCH_SCRIPT}" "${WORKFLOW_SCRIPT}" "${WORKFLOW_CONFIG}"

echo "Launching..."
echo "  lustre wrf workflow: ${WORKFLOW_CONFIG}"
echo "  job: $jid1"

