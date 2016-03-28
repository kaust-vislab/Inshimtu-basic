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
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace inshimtu
{
namespace options
{
const po::variables_map handleOptions(int argc, const char* const argv[]);

const std::vector<fs::path> collectScripts(const po::variables_map& opts);

const std::vector<fs::path> collectInitialFiles(const po::variables_map& opts);

const fs::path getWatchDirectory(const po::variables_map& opts);

const fs::path getDoneFile(const po::variables_map& opts);

const boost::regex getFileFilter(const po::variables_map& opts);

const uint getStartupDelay(const po::variables_map& opts);
}
}

#endif

