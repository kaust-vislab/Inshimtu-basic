/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#include "utils/help.h"

#include <iostream>
#include <string>

#include <cstdint>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

namespace inshimtu
{
namespace help
{
const uint32_t inshimtu_version_major = 0;
const uint32_t inshimtu_version_minor = 2;
const uint32_t inshimtu_version_patch = 0;


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

}
}
