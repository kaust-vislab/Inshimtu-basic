#Inshimtu

An In-Situ-Coprocessing-Shim between simulation output files (netCDF, HDF5, vkti, etc) and visualization pipelines (Catalyst).

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
module add kvl-applications paraview
paraview &
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

With Inshimtu running, connect via Catalyst in ParaView:

* Select Catalyst / Connect... from menu.
* Click OK in Catalyst Server Port dialog to accept connections from Inshimtu.
* Wait for connection to establish.
 
To demonstrate, copy the data files into the input directory (to simulate their creation via simulation):

```
unalias cp
cp -v build/original/*.vti build/testing/
touch build/testing.done
```

While the file creation (copying) is being performed, do the following in ParaView:

* Toggle Disclosure rectangle on catalyst/PVTrivialProducer1 source in Pipeline Browser to view data.
* Click Apply button for Extract:PVTrivialProducer1 filter.
* Make Extract:PVTrivialProducer1 filter visible.

Note: Alternatively, specify the files to process via the --initial files option, shown above.\n\ 


