#Inshimtu

An In-Situ-Coprocessing-Shim between simulation output files (netCDF, HDF5, vkti, etc) and visualization pipelines (Catalyst).

##Building

```
source setup.sh
cd build
cmake ..
```

## Running

*NOTE*: Currently input data directory is hard-coded (will fix in up-coming version)

Prepare the input data directory and completion notification file:

```
mkdir build/testing
touch build/testing.done
```

Run the application with the appropriate Catalyst viewer:

```
module add kvl-applications paraview
paraview &

build/Inshimtu testing/scripts/gridviewer.py &
```

With Inshimtu running, connect via Catalyst in ParaView:

* Select Catalyst / Connect... from menu.
* Click OK in Catalyst Server Port dialog to accept connections from Inshimtu.
* Wait for connection to establish.
 
To demonstrate, copy the data files into the input directory (to simulate their creation via simulation):

```
cp source_data_path/*.vti build/testing/
touch build/testing.done
```

While the file creation (copying) is being performed, do the following in ParaView:

* Toggle Disclosure rectangle on catalyst/PVTrivialProducer1 source in Pipeline Browser to view data.
* Click Apply button for Extract:PVTrivialProducer1 filter.
* Make Extract:PVTrivialProducer1 filter visible.


