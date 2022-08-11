#!/bin/bash

PROJECT_DIR="$(dirname "$(realpath "$0")")"

WORKFLOW_DIR="$PROJECT_DIR/workflow"
WORKFLOW_CONFIG="workflow.configs"

jid1=$(sbatch --time=30:00 --nodes=1 \
        "${WORKFLOW_DIR}/launch-workflow.sbat" \
          "${WORKFLOW_DIR}/run_workflow.sh" \
          "${PROJECT_DIR}/${WORKFLOW_CONFIG}" \
          "-i" \
      )
jid1=${jid1//[!0-9]/}

echo "Initializing lustre..."
echo "  lustre wrf workflow: ${WORKFLOW_CONFIG}"
echo "  job: $jid1"
