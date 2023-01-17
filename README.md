# Inshimtu Basic

An In-Situ-Coprocessing-Shim between simulation output files (netCDF, vti, pvti, etc) and visualization pipelines (Catalyst). This is the basic version of inshimtu that works based on files, so is great for testing in situ visualization with a simulation code requiring minimal/no changes to the simulation. This then allows simulation users to see what in situ can do so that they can decide if a full in situ integration should be done in their code or not.
Note: Inshimtu is an experimental tool developed to prototype specific use cases in a general way.  Expect that additional customization will be required to work with new (or even slightly modified) use cases.  Even with similar usecases (e.g., WRF + netCDF files), there can be hard-coded constraints (like data type and dimensionality) that might require code modification.
Pull requests that improve the generalizability of Inshimtu, or add new functionality while preserving generality, are welcome.


## Building

### Install boost 1.67.0

```
wget https://boostorg.jfrog.io/artifactory/main/release/1.67.0/source/boost_1_67_0.tar.gz
tar -xvf boost_1_67_0.tar.gz
cd boost_1_67_0/
./bootstrap.sh --with-python=python3
./b2 -j <num_procs>
```

### Paraview (Version 5.9.1)

note: I had to do a 'sudo apt install libgl1-mesa-dev' on a clean Ubuntu 20 for this to work

```
mkdir paraview
cd paraview
git clone --recursive https://gitlab.kitware.com/paraview/paraview-superbuild.git
cd paraview-superbuild
git fetch origin 
git checkout v5.9.1
git submodule update
cd ..
mkdir build
cd build
cmake ../paraview-superbuild -DENABLE_hdf5=ON -DENABLE-catalyst=ON -DENABLE_mpi=ON -DENABLE_python3=ON -DENABLE_netcdf=ON -DUSE_SYSTEM_python3=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j <num_procs>
```

### Install inshimtu

```
git clone https://gitlab.kaust.edu.sa/kvl/Inshimtu.git
cd Inshimtu
mkdir build && cd build
export ParaView_DIR="path/to/paraview-build/install"
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBoost_INCLUDE_DIR="path/to/boost_1_67_0"  ..
make -j <num_procs>

```



## Testing

```
ctest
```


## Running

Prepare the input data directory and completion notification file in the build directory:

```
mkdir testing
touch testing.done
```

Run the application with the appropriate Catalyst viewer and enable Catalyst connection in ParaView:

* Select Catalyst / Connect... from menu.
* Click OK in Catalyst Server Port dialog to accept connections from Inshimtu.
* Click Ok in Ready for Catalyst Connections dialog.
* Select Catalyst / Pause Simulation from menu.
* Wait for connection to establish, and follow the post-connection steps below.

Note: Failure to pause the simulation will prevent the first file from displaying.

 
The environment that runs Inshimtu requires the same ParaView environment it was built with, plus the ParaView Python libraries.

### Filtered Inshimtu
* Processes only newly created files matching regex '\w*(.vti)' in build/testing;
* Stops when build/testing.done file is touched;
* Uses the Catalyst script in pngQVAPOR.py to transfer data to ParaView.

```
./Inshimtu -w testing -d testing.done -s ../testing/pipelines/pngQVAPOR.py -f '\w*(.pvti)' -v QVAPOR -V trace
```
or
```
./Inshimtu -c ../testing/configs/png_watchDir_QVAPOR.json -V trace
```

To demonstrate, copy the data files into the input directory (to simulate their creation via simulation):

```
cp -v testing/data/wrf/* build/testing/
touch build/testing.done
```

Note: Alternatively, specify the files to process via the --initial files option, shown below in the json script.


### Pre-existing files processed with Inshimtu
* Processes all existing files specified in the script;
* Stops when all prexisting files are processed
* Uses the Catalyst script in pngQVAPOR.py to transfer data to ParaView.
* First copy wall wrf/pvti/* data to testing dir

```
./Inshimtu -c ../testing/configs/png_enumerated_QVAPOR.json -V trace
```


### Processing multiple variables with Inshimtu
* Processes only newly created files matching regex '\w*(.vtr)' in build/testing;
* Stops when build/testing.done file is touched;
* Uses the Catalyst scripts pngUVWQVAPOR.py and gridwriter.py to transfer data to ParaView.

#### View the selected variable in ParaView and write png

```
./Inshimtu -w testing -d testing.done -s ../testing/pipelines/pngUVWQVAPOR.py -f '\w*(.vtr)' -V trace -v U V W QVAPOR
```


#### Output the selected variables to disk:
Two different examples of how to do this.


#### Output variables as a single file

```
./Inshimtu -w testing -d testing.done -s ../testing/pipelines/gridwriter.py -f '\w*(.vtr)' -V trace -v U,V,W,QVAPOR
```


#### Output variables as seperate files

```
./Inshimtu -w testing -d testing.done -s ../testing/pipelines/gridwriter.py -f '\w*(.vtr)' -V trace -v U V W QVAPOR
```


## TODOs

* Schedule tasks (currently all inport nodes participate; works for Catalyst, but not for external commands)
  * Need coordination combinators

* Better handle how files are sent through the pipeline (more that 'input' 'output'); need to specify files that are 'finished', but not part of the input for the next stage.
  * Finish output specifications (what are they needed for? pipeline attribute adjustments)

* Fix how delete files are processed

* Fix Catalyst Pipeline singleton (need per script spec pipelines; or, at least per unique variable set)


## Notes

For WRF: see README.namelist
  &time_control
  ;; Works
  output_ready_flag = .true.,  ; asks the model to write-out an empty file with the name 'wrfoutReady_d<domain>_<date>.

  ;; Works
  history_outname = 'wrfout_d<domain>_<date>.nc' ; you may change the output file name

