/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#ifndef OPTIONS_HEADER
#define OPTIONS_HEADER

#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

namespace po = boost::program_options;
namespace pt = boost::property_tree;
namespace fs = boost::filesystem;

class Configuration
{
public:

  Configuration(int argc, const char* const argv[]);
  virtual ~Configuration();

  const std::vector<fs::path> collectScripts() const;

  const std::vector<fs::path> collectInitialFiles() const;

  const std::vector<std::string> collectVariables() const;

  const std::vector<std::pair<int, int>> collectInporterNodes() const;

  const bool hasWatchDirectory() const;
  const fs::path getWatchDirectory() const;

  const bool hasDoneFile() const;
  const fs::path getDoneFile() const;

  const bool hasFileFilter() const;
  const boost::regex getFileFilter() const;

  const uint getStartupDelay() const;

  const bool getDeleteFilesFlag() const;

protected:
  po::variables_map opts;
  pt::ptree configs;
};

#endif

