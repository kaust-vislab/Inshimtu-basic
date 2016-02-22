/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "application.h"
#include "notification.h"
#include "inporter.h"
#include "adaptor.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace
{
const uint32_t inshimtu_version_major = 0;
const uint32_t inshimtu_version_minor = 0;
const uint32_t inshimtu_version_patch = 2;


const po::variables_map handleOptions(int argc, const char* const argv[])
{
  po::variables_map opts;

  po::options_description basicDesc("basic usage options");
  basicDesc.add_options()
    ("help,h", "help message describing command line options")
    ("version,v", "output version number")
    ("watch,w", po::value<std::string>()->required(), "pre-existing inporting source directory to watch")
    ("files,f", po::value<std::string>()->default_value(".*"), "regular expression of watch directory files to process (ensure expression is 'quoted')")
    ("done,d", po::value<std::string>()->required(), "pre-existing termination trigger; done file; file must be outside watch directory")
    ("initial,i", po::value<std::vector<std::string>>()->multitoken()->default_value(std::vector<std::string>(), "<none>"), "space-seprated list of pre-exisiting files to process (unquoted for shell expansion)")
    ("scripts,s", po::value<std::vector<std::string>>()->required(), "list of Catalyst scripts for visualization processing")
    ("pause,p", po::value<uint>()->default_value(0), "initial delay in seconds to wait for ParaView to connect before processing commences")
   ;

  po::options_description helpDesc("help options");
  helpDesc.add_options()
    ("help-verbose", "explanatory help message")
    ("help-example", "example usage")
   ;

  po::options_description optionsDesc("command line options");
  optionsDesc.add(basicDesc).add(helpDesc);

  po::store(po::parse_command_line(argc, argv, optionsDesc), opts);


  bool foundHelp = false;
  bool foundVersion = false;

  if (opts.find("version") != opts.end())
  {
    std::cout << "Inshimtu v"
              << inshimtu_version_major << "."
              << inshimtu_version_minor << "."
              << inshimtu_version_patch
              << std::endl << std::endl;
    foundVersion = true;
  }


  if (opts.find("help-verbose") != opts.end())
  {
    std::cout << "Inshimtu is an In-Situ-Coprocessing-Shim between simulation output files (netCDF, HDF5, vkti, etc) and visualization pipelines (Catalyst)."
              << std::endl << std::endl
              << "The 'inporting' process watches an existing directory (--watch) for inotify changes, "
                 "reads in  modified files matching the filter (--files) after they are closed, "
                 "imports them into appropriate VTK formats, "
                 "and then exports them via Catalyst scripts (--scripts) to ParaView. "
                 "This process continues until the termination file (--done) is touched."
              << std::endl << std::endl;
    foundHelp = true;
  }

  if (opts.find("help-example") != opts.end())
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
                 "To demonstrate, copy the data files into the input directory (to simulate their creation via simulation): \n"
                 "\n"
                 "``` \n"
                 "unalias cp \n"
                 "cp -v build/original/*.vti build/testing/ \n"
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
    foundHelp = true;
  }

  if (foundHelp || opts.find("help") != opts.end())
  {
    std::cout << optionsDesc << std::endl;
    foundHelp = true;
  }



  if (foundVersion || foundHelp)
  {
    std::cout << std::flush;
    exit(1);
  }


  po::notify(opts);

  return opts;
}

const std::vector<fs::path> collectScripts(const po::variables_map& opts)
{
  std::vector<fs::path> scripts;

  if (opts.find("scripts") != opts.end())
  {
    const std::vector<std::string>& ss(opts["scripts"].as<std::vector<std::string>>());
    for (const std::string &sn : ss)
    {
      fs::path p(sn);
      scripts.push_back(fs::absolute(p));
    }
  }

  return scripts;
}

const std::vector<fs::path> collectInitialFiles(const po::variables_map& opts)
{
  std::vector<fs::path> files;

  if (opts.find("initial") != opts.end())
  {
    const std::vector<std::string>& ss(opts["initial"].as<std::vector<std::string>>());
    for (const std::string &sn : ss)
    {
      fs::path p(sn);
      files.push_back(fs::absolute(p));
    }
  }

  return files;
}

const fs::path getWatchDirectory(const po::variables_map& opts)
{
  return fs::absolute(fs::path(opts["watch"].as<std::string>()));
}

const fs::path getDoneFile(const po::variables_map& opts)
{
  return fs::absolute(fs::path(opts["done"].as<std::string>()));
}

const boost::regex getFileFilter(const po::variables_map& opts)
{
  return boost::regex(opts["files"].as<std::string>());
}

const uint getStartupDelay(const po::variables_map& opts)
{
  return opts["pause"].as<uint>();
}
}

int main(int argc, char* argv[])
{
  MPIApplication app(&argc, &argv);
  const po::variables_map opts(handleOptions(argc, argv));
  INotify notify( getWatchDirectory(opts)
                , getFileFilter(opts)
                , getDoneFile(opts));
  Catalyst catalyst( collectScripts(opts)
                   , getStartupDelay(opts));
  Inporter inporter(catalyst);
  std::vector<fs::path> newfiles(collectInitialFiles(opts));

  std::cout << "READY" << std::endl;

  do
  {
    if (newfiles.empty())
    {
      notify.processEvents(newfiles);
    }
    inporter.process(newfiles);
    newfiles.clear();
  } while (!notify.isDone());

  return 0;
}
