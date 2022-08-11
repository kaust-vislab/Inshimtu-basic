#!/bin/bash

# Note: if any step fails, what should be do? cancel everything
set -e

# run_workflow.sh <config_file>


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

function checkExecutable () {
  local name="$1"
  local value="${!name}"
  if [ -z "$name" ] ; then  
    echo "Invalid var name argument to checkExecutable"
    exit 1
  fi
  if [ -z "$value" ] || [ ! -x "$value" ] ; then  
    echo "Missing executable file ${name}: ${value}"
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

function checkNonEmpty () {
  local name="$1"
  local value="${!name}"
  if [ -z "$name" ] ; then  
    echo "Invalid var name argument to checkNonEmpty"
    exit 1
  fi
  if [ -z "$value" ] ; then  
    echo "Empty string ${name}: ${value}"
    exit 1
  fi
}

function checkVarSet () {
  local name="$1"
  local value="${!name}"
  if [ -z "$name" ] ; then  
    echo "Invalid var name argument to checkVarSet"
    exit 1
  fi
  if [ -z ${value+x} ] ; then  
    echo "Unset variable: ${name}"
    exit 1
  fi
}

function checkNumber () {
  local name="$1"
  local value="${!name}"
  if [ -z "$name" ] ; then  
    echo "Invalid var name argument to checkNumber"
    exit 1
  fi
  if [[ ! $value =~ ^-?[0-9]+$ ]] ; then  
    echo "Not a number ${name}: ${value}"
    exit 1
  fi
}

function checkPositiveNumber () {
  local name="$1"
  local value="${!name}"
  if [ -z "$name" ] ; then  
    echo "Invalid var name argument to checkPositiveNumber"
    exit 1
  fi
  if [[ ! $value =~ ^[0-9]+$ ]] ; then  
    echo "Not a positive number ${name}: ${value}"
    exit 1
  fi
}

function checkBoolean () {
  local name="$1"
  local value="${!name}"
  if [ -z "$name" ] ; then  
    echo "Invalid var name argument to checkBoolean"
    exit 1
  fi
  if [ "$value" != true ] && [ "$value" != false ] ; then  
    echo "Not a boolean ${name}: ${value}"
    exit 1
  fi
}

function checkFunction () {
  local name="$1"
  if [ -z "$name" ] ; then  
    echo "Invalid var name argument to checkFunction"
    exit 1
  fi
  if [ $(type -t $name) != 'function' ] ; then  
    echo "Not a function: ${name}"
    exit 1
  fi
}


export -f checkFile
export -f checkExecutable
export -f checkDirectory
export -f checkNonEmpty
export -f checkVarSet
export -f checkNumber
export -f checkPositiveNumber
export -f checkBoolean
export -f checkFunction


function usage () {
  echo "run_workflow.sh workflow.config [-i] [-h]"
  echo "  -i  initialize only"
  echo "  -h  help"
}


## INPUTS

WKFLOW_CONFIGS_FILE="$1"
checkFile WKFLOW_CONFIGS_FILE
shift

# Use getopts to get arguments
#
while getopts ":ih" opt; do
  case $opt in
    i)
      INIT_ONLY=true
      ;;
    h)
      usage
      exit 0
      ;;
    [?])
      echo "Invalid option: -$OPTARG" >&2;
      usage
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2;
      usage
      exit 1
      ;;
  esac
done
shift $((OPTIND-1))


## INIT

export WKFLOW_LAUNCH_DIR="$(dirname "$(realpath -s "$0")")"
checkDirectory WKFLOW_LAUNCH_DIR


## CONFIGS

echo "Sourcing WKFLOW_CONFIGS_FILE: $WKFLOW_CONFIGS_FILE"
source "$WKFLOW_CONFIGS_FILE"

checkExecutable INIT_SCRIPT
checkExecutable SIM_SCRIPT
checkExecutable FINISH_SCRIPT

checkDirectory WKFLOW_WORK_DIR
checkNonEmpty SIM_OUTPUT_DIR


## RUN

echo "Running INIT_SCRIPT: $INIT_SCRIPT"
"$INIT_SCRIPT"

checkDirectory SIM_OUTPUT_DIR

if [ "$INIT_ONLY" = true ] ; then
  echo "Initialization Complete INIT_ONLY: $INIT_ONLY"
  exit 0
fi

echo "Running SIM_SCRIPT: $SIM_SCRIPT"
"$SIM_SCRIPT"

echo "Running FINISH_SCRIPT: $FINISH_SCRIPT"
"$FINISH_SCRIPT"

