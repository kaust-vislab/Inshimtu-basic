#!/bin/bash

shopt -s extglob

# workflow.finalize.sh


## INPUTS

checkFunction checkDirectory
checkFunction checkFile
checkFunction checkExecutable
checkFunction checkNonEmpty
checkFunction checkVarSet
checkFunction checkNumber


checkDirectory SIM_OUTPUT_DIR


## SETUP

ls -alhFR "$SIM_OUTPUT_DIR"

echo cat "$SIM_OUTPUT_DIR/"rsl.error.+(0)
cat "$SIM_OUTPUT_DIR/"rsl.error.+(0)

