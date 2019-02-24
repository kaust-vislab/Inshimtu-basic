/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#ifndef CORE_OPTIONS_HEADER
#define CORE_OPTIONS_HEADER

#include "core/specifications.h"
#include "utils/logger.h"

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

// TODO: Fix reversed dependency by moving ProcessingSpecCommands into specifications
#include "processing/pipeline.h"


class Configuration
{
public:

  Configuration(int argc, const char* const argv[]);
  virtual ~Configuration();

  const std::vector<PipelineSpec> collectPipelines() const;

  const std::vector<ProcessingSpecCommands::Command> collectExternalCommands() const;

  const std::vector<boost::filesystem::path> collectInitialFiles() const;

  typedef std::pair<int, int> NodeRange;
  const std::vector<NodeRange> collectInporterNodes() const;

  bool hasWatchPaths() const;
  const std::vector<InputSpecPaths> getWatchPaths() const;

  bool hasDoneFile() const;
  const boost::filesystem::path getDoneFile() const;

  uint getStartupDelay() const;

  bool getDeleteFilesFlag() const;

  boost::optional<boost::log::trivial::severity_level> getVerbosity() const;

  const boost::filesystem::path& getLibPath() const;



  // TODO: deprecate when  calculateReadyFiles is part of process pipeline (not coordinator getSignalledOutputFile)
  const boost::optional<ReplaceRegexFormat> getOutputReadyConversion() const;
  // TODO: deprecate when Pipeline manages scripts in the new pipeline processing style
  const std::vector<boost::filesystem::path> collectScripts() const;


private:

  const std::vector<std::string> collectVariables() const;

  bool hasWatchDirectory() const;
  const boost::filesystem::path getWatchDirectory() const;

  bool hasFileFilter() const;
  const boost::regex getFileFilter() const;


protected:

  boost::program_options::variables_map opts;
  boost::property_tree::ptree configs;
  boost::filesystem::path libPath;
};

#endif

