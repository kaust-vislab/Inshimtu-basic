# Generic Inshimtu Workflows

Workflow steps:

* Stage - Input
  * Simulation Sources (e.g., namelist.txt, WRF/run directory components)
  * Inshimtu Sources (e.g., configs, scripts, pipelines, meta:layout)
  * Parameters:
    * Relative Paths (e.g., wrfout, wrfout.done)
    * Job Specs (# nodes total, # nodes for simulation, # nodes for inshimtu, # tasks / node for inshimtu)
* Stage - Initialize [job start]
  * Initialize Burst Buffer / Lustre working directories
  * Stage-in / copy / link input source files
  * Create output directories (as required)
* Stage - Resource Allocation : wait <- for initialize to complete >>= start job
* Stage - Update
  * Copy pipeline (if destination differs from source)
  * Copy / link simulation files (as required)
  * Create output directories (as required)
* Stage - Run Inshimtu (ROOT_DIR, PATHS, JSPECS)
  * module loads
  * Specify total nodes
  * Specify nodes for inport / processing
  * Launch inshimtu
  * background
* Stage - Run Simulation (ROOT_DIR, SIM_PATHS, JSPECS)
  * module loads
  * Specify (#) nodes for simulation
  * Specify available core / node
  * Launch simulation
  * wait
* Stage - Completion
  * Notify done
* [Stage - Stage-out]
  * Copy visualization / analysis files to Lustre

Workflow paths:

* WKFLOW_PROJECT_DIR
  * Path contains project specific source scripts and configurations
  * Source directories (simulation inputs, pipelines, initialization scripts) are relative to this path
    or are at known absolutes
* WKFLOW_WORK_DIR
  * The top-level destination directory for runtime output
  * Self-contained directory contains pipelines, simulation input, simulation output,
    and visualization / analytics output (possibly symlinked).
* WKFLOW_LAUNCH_DIR
  * Directory for the generic run_* workflow scripts
* WKFLOW_WORKDIR_INITIALIZE_REQ
  * Enum: none, copy, link 
  * Describes how initialize script should prepare the working directory (source and input)


## Basic


```
launch_workflow launch_wrf.sh launch_inshimtu.sh 
```


