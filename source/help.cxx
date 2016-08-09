/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "help.h"

#include <iostream>
#include <string>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

namespace inshimtu
{
namespace help
{
const uint32_t inshimtu_version_major = 0;
const uint32_t inshimtu_version_minor = 0;
const uint32_t inshimtu_version_patch = 3;


void printVersion()
{
  std::cout << "Inshimtu v"
            << inshimtu_version_major << "."
            << inshimtu_version_minor << "."
            << inshimtu_version_patch
            << std::endl << std::endl;
}

void printHelpVerbose()
{
  std::cout << "Inshimtu is an In-Situ-Coprocessing-Shim between simulation output files "
            << "(netCDF, HDF5, vkti, etc) and visualization pipelines (Catalyst)."
            << std::endl << std::endl
            << "The 'inporting' process watches an existing directory (--watch) for inotify changes, "
               "reads in  modified files matching the filter (--files) after they are closed, "
               "imports the specified variables (--variables) from them into appropriate VTK formats, "
               "and then exports them via Catalyst scripts (--scripts) to ParaView. "
               "This process continues until the termination file (--done) is touched."
            << std::endl << std::endl;
}

void printHelpExample()
{
  std::cout << "Inshimtu examples:"
            << std::endl << std::endl
            << "## Running \n"
               "Prepare the input data directory and completion notification file: \n"
               "\n"
               "``` \n"
               "mkdir build/testing \n"
               "touch build/testing.done \n"
               "``` \n"
               "\n"
               "Run the application with the appropriate Catalyst viewer: \n"
               "\n"
               "``` \n"
               "module add kvl-applications paraview/4.4.0-mpich-x86_64 \n"
               "paraview & \n"
               "``` \n"
               "\n"
               "Enable Catalyst connection in ParaView: \n"
               "* Select Catalyst / Connect... from menu. \n"
               "* Click OK in Catalyst Server Port dialog to accept connections from Inshimtu. \n"
               "* Click Ok in Ready for Catalyst Connections dialog. \n"
               "* Select Catalyst / Pause Simulation from menu. \n"
               "* Wait for connection to establish. \n"
               "\n"
               "Note: Failure to pause the simulation will prevent the first file from displaying. \n"
               "\n"
               "The environment that runs Inshimtu requires the same ParaView "
               "environment it was built with, plus the ParaView Python libraries. \n"
               "For now, use this module to update the PYTHONPATH: \n"
               "\n"
               "``` \n"
               "module add dev-inshimtu \n"
               "``` \n"
               "\n"
               "Basic Inshimtu: \n"
               "* Processes any newly created file in build/testing;\n"
               "* Stops when build/testing.done file is touched;\n"
               "* Uses the Catalyst script in gridviewer.py to transfer data to ParaView.\n"
               "``` \n"
               "build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py \n"
               "``` \n"
               "\n"
               "Filtered Inshimtu: \n"
               "* Processes only newly created files matching regex 'filename.*.vti' in build/testing;\n"
               "* Stops when build/testing.done file is touched;\n"
               "* Uses the Catalyst script in gridviewer.py to transfer data to ParaView.\n"
               "``` \n"
               "build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py -f 'filename.*.vti' \n"
               "``` \n"
               "\n"
               "Pre-existing files + Basic Inshimtu: \n"
               "* Processes all existing files in build/testing matching the 'filename*.vti' shell glob;\n"
               "* Processes any newly created files in build/testing;\n"
               "* Stops when build/testing.done file is touched;\n"
               "* Uses the Catalyst script in gridviewer.py to transfer data to ParaView.\n"
               "``` \n"
               "build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py -i build/testing/filename*.vti \n"
               "``` \n"
               "\n"
               "Variable Support: \n"
               "* Processes only specified variables via variable sets;\n"
               "* Stops when build/testing.done file is touched;\n"
               "* Uses the Catalyst script in gridviewer.py to transfer data to ParaView.\n"
               "``` \n"
               "build/Inshimtu -w build/testing -d build/testing.done -s testing/scripts/gridviewer.py -f 'wrfout*.nc' -v U,V,W,QVAPOR\n"
               "``` \n"
               "\n"
               "To demonstrate, copy the data files into the input directory (to simulate their creation via simulation): \n"
               "\n"
               "``` \n"
               "\\cp -v build/original/*.vti build/testing/ \n"
               "touch build/testing.done \n"
               "``` \n"
               "\n"
               "Note: Alternatively, specify the files to process via the --initial files option, shown above.\n"
               "\n"
               "Post-Connection: While the file creation (copying) is being performed, do the following in ParaView: \n"
               "\n"
               "* Toggle Disclosure rectangle on catalyst/PVTrivialProducer1 source in Pipeline Browser to view data. \n"
               "* Click Apply button for Extract:PVTrivialProducer1 filter. \n"
               "* Make Extract:PVTrivialProducer1 filter visible.\n"
               "* Set Variable and Representation. \n"
               "* Select Catalyst / Continue from menu.\n"
            << std::endl;
}

}
}
