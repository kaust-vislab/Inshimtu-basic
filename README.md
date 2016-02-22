#Inshimtu

An In-Situ-Coprocessing-Shim between simulation output files (netCDF, HDF5, vkti, etc) and visualization pipelines (Catalyst).

##TODOs
### Testing Example...
build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py -i /lustre/project/k1033/Projects/hari/makkah2_wrfout_d02_2015.qsnow_qrain_qvapor.nc

build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py -i build/testing/filename_1_0.vti

build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py -i build/testing/filename_*.vti

### Notes



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

 
The environment that runs Inshimtu requires the same ParaView environment it was built with, plus the ParaView Python libraries.  For now, use this module to update the PYTHONPATH:

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

To demonstrate, copy the data files into the input directory (to simulate their creation via simulation):

```
unalias cp
cp -v build/original/*.vti build/testing/
touch build/testing.done
```

Post-Conntection: While the file creation (copying) is being performed, do the following in ParaView:

* Toggle Disclosure rectangle on catalyst/PVTrivialProducer1 source in Pipeline Browser to view data.
* Click Apply button for Extract:PVTrivialProducer1 filter.
* Make Extract:PVTrivialProducer1 filter visible.
* Set Variable and Representation
* Select Catalyst / Continue from menu.

Note: Alternatively, specify the files to process via the --initial files option, shown above.\n\ 


