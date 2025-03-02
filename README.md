# Inshimtu Basic

An In-Situ-Coprocessing-Shim between simulation output files (netCDF, vti, pvti, etc) and visualization pipelines (Catalyst). This is the basic version of inshimtu that works based on files, so is great for testing in situ visualization with a simulation code requiring minimal/no changes to the simulation. This then allows simulation users to see what in situ can do so that they can decide if a full in situ integration should be done in their code or not.

Note: Inshimtu is an experimental tool developed to prototype specific use cases in a general way.  Expect that additional customization will be required to work with new (or even slightly modified) use cases.  Even with similar usecases (e.g., WRF + netCDF files), there can be hard-coded constraints (like data type and dimensionality) that might require code modification.

Pull requests that improve the generalizability of Inshimtu, or add new functionality while preserving generality, are welcome.


## Building


### Install boost 1.84.0

```
wget https://archives.boost.io/release/1.85.0/source/boost_1_85_0.tar.gz
tar -xvf boost_1_85_0.tar.gz
cd boost_1_85_0/
./bootstrap.sh --with-python=python3
./b2 -j <num_procs>
```

### Paraview (Version master)

note: I had to do a `sudo apt install libgl1-mesa-dev` on a clean Ubuntu 20 for this to work

```
mkdir paraview
cd paraview
git clone --recursive https://gitlab.kitware.com/paraview/paraview-superbuild.git
cd paraview-superbuild
git fetch origin 
git checkout master
git submodule update
cd ..
mkdir build
cd build
cmake ../paraview-superbuild -DENABLE_hdf5=ON -DENABLE_catalyst=ON -DENABLE_mpi=ON -DENABLE_python3=ON -DENABLE_netcdf=ON -DUSE_SYSTEM_python3=ON -DUSE_SYSTEM_mpi=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j <num_procs>
```

### VTK (9.2.6)

We will use the VTK built by ParaView. In ParaView versions prior to 5.13 building a seperate VTK was necessary, so that is no longer the case. 


### Install inshimtu

```
git clone https://github.com/kaust-vislab/Inshimtu-basic/
cd Inshimtu-basic
mkdir build && cd build
export ParaView_DIR=<path/to/paraview-build/install>
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -Dcatalyst_DIR=<path/to/catalyst-2.0> \
      -DVTK_DIR=<path/to/cmake/paraview-5.13/vtk>  \
      -DBOOST_ROOT=<path/to/boost_1_85_0> \
      -DBoost_NO_BOOST_CMAKE=TRUE \
      -DBoost_NO_SYSTEM_PATHS=TRUE \
      make -j <num_procs> ../

```


## Running

Prepare the input data directory and completion notification file in the build directory:

```
mkdir testing
touch testing.done
```

Run the application with the appropriate Catalyst viewer and enable Catalyst connection in ParaView:

* Select ``Catalyst / Connect...`` from menu.
* Click ``OK`` in Catalyst Server Port dialog to accept connections from Inshimtu.
* Click ``OK`` in Ready for Catalyst Connections dialog.
* Select ``Catalyst / Pause Simulation`` from menu.
* Wait for connection to establish, and follow the post-connection steps below.

Note: Failure to pause the simulation will prevent the first file from displaying.

 
The environment that runs Inshimtu requires the same ParaView environment it was built with, plus the ParaView Python libraries.

### Filtered Inshimtu
* Processes only newly created files matching regex ``'\w*(.pvti)'`` in ``build/testing``;
* Stops when ``build/testing.done`` file is touched;
* Uses the Catalyst script in ``pngQVAPOR.py`` to transfer data to ParaView.
* Make sure to use the correct path for your local Catalyst library

```
./inshimtu -w testing -d testing.done -s ../testing/pipelines/png_writer_QVAPOR.py -f '\w*(.pvti)' -variables QVAPOR -verbosity trace --catalyst_lib /home/kressjm/packages/inshimtu-catalystV2/paraview/build-513/install/lib/catalyst
```
or
```
./inshimtu -c ../testing/configs/png_watchDir_QVAPOR.json --verbosity trace
```

To demonstrate, copy the data files into the input directory (to simulate their creation via simulation):

```
cp -rv ../testing/data/wrf/pvti/* testing/
touch build/testing.done
```

Note: Alternatively, specify the files to process via the ``--initial`` files option, shown below in the json script.


### Pre-existing files processed with Inshimtu
* Processes all existing files specified in the script;
* Stops when all prexisting files are processed
* Uses the Catalyst script `png_writer_QVAPOR.py` to transfer data to ParaView.
* Make sure to use the correct path for your local Catalyst library

```
./inshimtu -c ../testing/configs/png_enumerated_QVAPOR.json --verbosity trace
```

#### Processing a variable and then writing the data set to disk with Inshimtu
* Extracts QVAPOR and writes it to disk

```
./inshimtu -c ../testing/configs/gridwriter_enumerated.json --verbosity trace
```


### Processing multiple variables with Inshimtu
* Processes only newly created files matching regex '\w*(.vtr)' in build/testing;
* Stops when build/testing.done file is touched;
* Uses the Catalyst scripts ``gridwriter.py`` and ``png_writer_clip_wv.py`` to transfer data to ParaView.
* Make sure to use the correct path for your local Catalyst library

#### View the selected variable in ParaView and write png

```
./inshimtu -c ../testing/configs/png_watchDir_gray-scott.json --verbosity trace
```


#### Output the selected variables to disk:
Two different examples of how to do this.


#### Output variables as a single file
* Writes the variables ``u,v`` to a single file.
* This is specified by having the variables comma seperated, below you will see what happens when they are space seperated.
* Make sure to use the correct path for your local Catalyst library

```
./inshimtu -c ../testing/configs/gridwriter_enumerated-gray-scott.json --verbosity trace
```

Alernatively, you can run without the config script and specify everything on the command line:

* Copy the Gray-Scott files to the ``testing`` dir once Inshimtu is running.
```
./inshimtu --variables u,v -s ../testing/pipelines/gridwriter.py -w testing -d testing.done -f ".*_step-\\d+\\.pvti" --verbosity trace --catalyst_lib /home/kressjm/packages/inshimtu-catalystV2/paraview/build-513/install/lib/catalyst
```


#### Output variables as seperate files
* Writes the variables ``u v`` to individual files.
* This is specified by the variables being space seperated.
* Copy the Gray-Scott files to the ``testing`` dir once Inshimtu is running.
* Make sure to use the correct path for your local Catalyst library

```
./inshimtu -c ../testing/configs/gridwriter_watchDir-gray-scott.json --verbosity trace
```

Alernatively, you can run without the config script and specify everything on the command line:

```
./inshimtu --variables u v  -s ../testing/pipelines/gridwriter.py -w testing -d testing.done -f ".*_step-\\d+\\.pvti" --verbosity trace --catalyst_lib /home/kressjm/packages/inshimtu-catalystV2/paraview/build-513/install/lib/catalyst
```


### Running with a live simulation
You can download and build the Gray-Scott miniapp that we have packaged in a seperate repo if you want to have a live data source to use with Inshimtu. 

* Download KAUST Visualization Vignettes: 
  * https://gitlab.kitware.com/jameskress/KAUST_Visualization_Vignettes.git
* Build following the repo instructions: 
  * https://gitlab.kitware.com/jameskress/KAUST_Visualization_Vignettes/-/blob/master/Miniapps/gray-scott/README.md?ref_type=heads
* Setup the Inshimtu script
  * Modify ``gridwriter-watchDir-gray-scott.json
  * Change the ``directory_path`` to where you will run Gray-Scott
  * Make sure to use the correct path for your local Catalyst library
* Run Inshimtu
* Connect in ParaView if you want to watch live
* Run Gray-Scott with the pvti writer following these instructions
  * https://gitlab.kitware.com/jameskress/KAUST_Visualization_Vignettes/-/blob/master/Miniapps/gray-scott/README.md?ref_type=heads#how-to-run-with-pvti-writer  
  * If the simulation runs too fast modify the settings file to make the grid larger and/or run for more steps
