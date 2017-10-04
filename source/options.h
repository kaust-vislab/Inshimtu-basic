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


class Configuration
{
public:

  Configuration(int argc, const char* const argv[]);
  virtual ~Configuration();

  const std::vector<boost::filesystem::path> collectScripts() const;

  const std::vector<boost::filesystem::path> collectInitialFiles() const;

  const std::vector<std::string> collectVariables() const;

  typedef std::pair<int, int> NodeRange;
  const std::vector<NodeRange> collectInporterNodes() const;

  const bool hasWatchDirectory() const;
  const boost::filesystem::path getWatchDirectory() const;

  const bool hasDoneFile() const;
  const boost::filesystem::path getDoneFile() const;

  const bool hasFileFilter() const;
  const boost::regex getFileFilter() const;

  typedef std::pair<boost::regex, std::string> ReplaceRegexFormat;
  const bool hasOutputReadySignal() const;
  const boost::optional<ReplaceRegexFormat> getOutputReadyConversion() const;

  const uint getStartupDelay() const;

  const bool getDeleteFilesFlag() const;

protected:
  boost::program_options::variables_map opts;
  boost::property_tree::ptree configs;
};

#endif

