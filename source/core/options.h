/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#ifndef CORE_OPTIONS_HEADER
#define CORE_OPTIONS_HEADER

#include "core/specifications.h"

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


class Configuration
{
public:

  Configuration(int argc, const char* const argv[]);
  virtual ~Configuration();

  const std::vector<boost::filesystem::path> collectScripts() const;
  const std::vector<boost::filesystem::path> collectExternalCommands() const;

  const std::vector<boost::filesystem::path> collectInitialFiles() const;

  const std::vector<std::string> collectVariables() const;

  typedef std::pair<int, int> NodeRange;
  const std::vector<NodeRange> collectInporterNodes() const;

  bool hasWatchDirectory() const;
  const boost::filesystem::path getWatchDirectory() const;

  bool hasWatchPaths() const;
  const std::vector<InputSpecPaths> getWatchPaths() const;

  bool hasDoneFile() const;
  const boost::filesystem::path getDoneFile() const;

  bool hasFileFilter() const;
  const boost::regex getFileFilter() const;

  bool hasOutputReadySignal() const;
  const boost::optional<ReplaceRegexFormat> getOutputReadyConversion() const;

  uint getStartupDelay() const;

  bool getDeleteFilesFlag() const;


  const boost::filesystem::path& getLibPath() const;


protected:
  boost::program_options::variables_map opts;
  boost::property_tree::ptree configs;
  boost::filesystem::path libPath;
};

#endif

