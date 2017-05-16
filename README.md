#Inshimtu

An In-Situ-Coprocessing-Shim between simulation output files (netCDF, HDF5, vkti, etc) and visualization pipelines (Catalyst).

##TODOs
### Testing Example...

module add kvl-applications paraview/5.3.0-mpich-x86_64

#### Example of running on GPGPU with 16 nodes total, and 9 nodes as inporters (extent import + Catalyst)
time testing/launchers/launch_inshimtu_kvl.sh -S GDM -n 0-8 -N 1-16


build/Inshimtu -w build/testing -d build/testing.done -s testing/pipelines/gridviewer.py -i build/testing/filename_*.vti -v input

build/Inshimtu -w build/testing -d build/testing.done -s testing/pipelines/gridviewer.py -i build/testing/WSM3/WSM3_wrfout_d01_2015-*.nc -v U,V,W,QVAPOR

build/Inshimtu -w build/testing -d build/testing.done -s testing/pipelines/gridviewer.py -i /lustre/project/k1033/Projects/hari/makkah2_wrfout_d02_2015.qsnow_qrain_qvapor.nc

build/Inshimtu -w build/testing -d build/testing.done -s testing/pipelines/gridviewer.py -i build/testing/filename_1_0.vti

build/Inshimtu -w build/testing -d build/testing.done -s testing/pipelines/gridviewer.py -i build/testing/filename_*.vti


srun --ntasks-per-node=1 "$INSHIMTU_DIR/Inshimtu" \
     -w "$OUTPUT_ROOT/wrfrun-output" -d "$OUTPUT_ROOT/wrfrun-output.done" \
     -i "${OUTPUT_ROOT}/wrfrun-output/"wrfout_d*.nc \
     -s "$INSHIMTU_ROOT/testing/pipelines/gridviewer.py" \
     -v U,V,W,QVAPOR
  &

# single node
module add mpi/mpich-x86_64
cd /home/holstgr/Development/Inshimtu/examples
../build.kvl/Inshimtu -w "$PWD/testing/GDM" -d "$PWD/testing.done" \
                      -i "$PWD/testing/GDM/"wrfout_d01_* \
                      -s "$PWD/pipelines/viewer_P_QICE_QVAPOR.py" \
                      -v QVAPOR,P,QICE

../build.kvl/Inshimtu \
    -w "$PWD/testing/GDM" -d "$PWD/testing.done" \
    -i "$PWD/testing/GDM/"wrfout_d01_* \
    -s "$PWD/pipelines/viewer_QVAPOR.py" -v QVAPOR

# note: much slower with decimate
"$PWD/../build.kvl/Inshimtu" \
    -w "$PWD/testing/GDM" -d "$PWD/testing.done" \
    -i "$PWD/testing/GDM/"wrfout_d01_* \
    -s "$PWD/pipelines/viewer_P_QICE.py" -v P,QICE

"$PWD/../build.kvl/Inshimtu" \
    -w "$PWD/testing/GDM" -d "$PWD/testing.done" \
    -i "$PWD/testing/GDM/"wrfout_d01_* \
    -s "$PWD/pipelines/viewer_P_QICE_nodec.py" -v P,QICE


# MPI multi node
module add kvl-applications paraview/5.3.0-mpich-x86_64
cd /home/holstgr/Development/Inshimtu/examples
mpirun -np 8 -hosts gpgpu-01,gpgpu-02,gpgpu-03,gpgpu-04,gpgpu-05,gpgpu-06,gpgpu-07,gpgpu-08 \
  "$PWD/../build.kvl/Inshimtu" \
    -w "$PWD/testing/GDM" -d "$PWD/testing.done" -i "$PWD/testing/GDM/"wrfout_d01_* \
    -s "$PWD/pipelines/viewer_P_QICE_QVAPOR.py" -v QVAPOR,P,QICE

mpirun -np 8 -hosts gpgpu-01,gpgpu-02,gpgpu-03,gpgpu-04,gpgpu-05,gpgpu-06,gpgpu-07,gpgpu-08 \
  "$PWD/../build.kvl/Inshimtu" \
    -w "$PWD/testing/GDM" -d "$PWD/testing.done" \
    -i "$PWD/testing/GDM/"wrfout_d01_2015-{10-28,10-29,10-30,10-31,11-01}* \
    -s "$PWD/pipelines/viewer_P_QICE_nodec.py" -v P,QICE


### Notes

For WRF: see README.namelist
  &time_control
  ;; Works
  output_ready_flag = .true.,  ; asks the model to write-out an empty file with the name 'wrfoutReady_d<domain>_<date>.

  ;; Works
  history_outname = 'wrfout_d<domain>_<date>.nc' ; you may change the output file name

##Building

```
./setup.sh
## Peforms:
# module add kvl-applications paraview
# cd build
# cmake ..
```

## Running

Prepare the input data directory and completion notification file:

```
mkdir build/testing
touch build/testing.done
```

Run the application with the appropriate Catalyst viewer:

```
module add kvl-applications paraview/4.4.0-mpich-x86_64
paraview &
```

Enable Catalyst connection in ParaView:

* Select Catalyst / Connect... from menu.
* Click OK in Catalyst Server Port dialog to accept connections from Inshimtu.
* Click Ok in Ready for Catalyst Connections dialog.
* Select Catalyst / Pause Simulation from menu.
* Wait for connection to establish, and follow the post-connection steps below.

Note: Failure to pause the simulation will prevent the first file from displaying.

 
The environment that runs Inshimtu requires the same ParaView environment it was built with, plus the ParaView Python libraries.  PYTHONPATH is set in the paraview module.

```
module add dev-inshimtu
```

Basic Inshimtu:
* Processes any newly created file in build/testing;
* Stops when build/testing.done file is touched;
* Uses the Catalyst script in gridviewer.py to transfer data to ParaView

```
build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py
```

Filtered Inshimtu:
* Processes only newly created files matching regex 'filename.*.vti' in build/testing;
* Stops when build/testing.done file is touched;
* Uses the Catalyst script in gridviewer.py to transfer data to ParaView.
```
build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py -f 'filename.*.vti'
```

Pre-existing files + Basic Inshimtu:
* Processes all existing files in build/testing matching the 'filename*.vti' shell glob;
* Processes any newly created files in build/testing;
* Stops when build/testing.done file is touched;
* Uses the Catalyst script in gridviewer.py to transfer data to ParaView.
```
build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py -i build/testing/filename*.vti
```

Variable Support: 
* Processes only specified variables via variable sets;
* Stops when build/testing.done file is touched;
* Uses the Catalyst script in gridviewer.py to transfer data to ParaView.
``` 
build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py -f 'wrfout*.nc' -v U,V,W,QVAPOR
``` 

To demonstrate, copy the data files into the input directory (to simulate their creation via simulation):

```
# \cp is equivalent to unaliased cp
\cp -v build/original/*.vti build/testing/
touch build/testing.done
```

Post-Connection: While the file creation (copying) is being performed, do the following in ParaView:

* Toggle Disclosure rectangle on catalyst/PVTrivialProducer1 source in Pipeline Browser to view data.
* Click Apply button for Extract:PVTrivialProducer1 filter.
* Make Extract:PVTrivialProducer1 filter visible.
* Set Variable and Representation
* Select Catalyst / Continue from menu.

Note: Alternatively, specify the files to process via the --initial files option, shown above.



