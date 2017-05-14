/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "options.h"
#include "help.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

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
const po::variables_map handleOptions(int argc, const char* const argv[])
{
  po::variables_map opts;

  po::options_description basicDesc("basic usage options");
  basicDesc.add_options()
    ("help,h", "help message describing command line options")
    ("version,V", "output version number")
    ("watch,w", po::value<std::string>()->default_value(""), "pre-existing inporting source directory to watch")
    ("done,d", po::value<std::string>()->default_value(""), "pre-existing termination trigger; done file; file must be outside watch directory")
    ("files,f", po::value<std::string>()->default_value(""), "regular expression of watch directory files to process (ensure expression is 'quoted')")
    ("initial,i", po::value<std::vector<std::string>>()->multitoken()->default_value(std::vector<std::string>(), "<none>"), "space-separated list of pre-exisiting files to process (unquoted for shell expansion)")
    ("scripts,s", po::value<std::vector<std::string>>()->required(), "list of Catalyst scripts for visualization processing")
    ("variables,v", po::value<std::vector<std::string>>()->multitoken()->default_value(std::vector<std::string>(), "<none>"), "space-separated list of comma-separated variable sets to process")
    ("nodes,n", po::value<std::string>()->default_value(""), "comma-separated list of node-id intervals specifying inporter Catalyst nodes")
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
    inshimtu::help::printVersion();
    foundVersion = true;
  }


  if (opts.find("help-verbose") != opts.end())
  {
    inshimtu::help::printHelpVerbose();
    foundHelp = true;
  }

  if (opts.find("help-example") != opts.end())
  {
    inshimtu::help::printHelpExample();
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

  // validate
  {
    const bool watchDirectory(hasWatchDirectory(opts));
    const bool doneFile(hasDoneFile(opts));
    const bool fileFilter(hasFileFilter(opts));
    const bool initialFiles(!collectInitialFiles(opts).empty());

    if (initialFiles)
    {
      if ((watchDirectory && !doneFile) || (!watchDirectory && doneFile))
      {
        throw std::runtime_error("either specify both options '--watch' and '--done', or specify neither when there are '--initial' files");
      }

      if (!watchDirectory && !doneFile && fileFilter)
      {
        throw std::runtime_error("the specified option '--files' requires both options '--watch' and '--done'");
      }
    }
    else
    {
      if (!watchDirectory)
      {
        throw std::runtime_error("the missing option '--watch' is required when there are no '--initial' files");
      }

      if (!doneFile)
      {
        throw std::runtime_error("the missing option '--done' is required when there are no '--initial' files");
      }
    }
  }

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

const std::vector<std::string> collectVariables(const po::variables_map& opts)
{
  std::vector<std::string> vars;

  if (opts.find("variables") != opts.end())
  {
    vars = opts["variables"].as<std::vector<std::string>>();
  }

  return vars;
}

const std::vector<std::pair<int, int>> collectInporterNodes(const po::variables_map& opts)
{
  // intervals are node id pairs <s,e> specifying the start through end nodes inclusive
  std::vector<std::pair<int, int>> nintervals;
  std::vector<std::string> ivals;
  const std::string ivalstr(opts["nodes"].as<std::string>());
  const boost::regex regexNums("(\\d+)-(\\d+)|(\\d+)");

  boost::split(ivals, ivalstr, boost::is_any_of(","), boost::token_compress_on);

  for (const auto& iv : ivals)
  {
    boost::smatch what;

    if (boost::regex_search(iv, what, regexNums))
    {
      const bool single = what[3].matched;
      int n1 = single ? boost::lexical_cast<int>(what[3]) : boost::lexical_cast<int>(what[1]);
      int n2 = single ? boost::lexical_cast<int>(what[3]) : boost::lexical_cast<int>(what[2]);

      nintervals.push_back(std::pair<int,int>(n1, n2));
    }
    else
    {
      throw std::runtime_error("the option '--nodes' must have interval format: ((#|(#-#)),)*(#|(#-#))");
    }
  }

  return nintervals;
}


const bool hasWatchDirectory(const po::variables_map& opts)
{
  return !opts["watch"].as<std::string>().empty();
}

const fs::path getWatchDirectory(const po::variables_map& opts)
{
  return fs::absolute(fs::path(opts["watch"].as<std::string>()));
}

const bool hasDoneFile(const po::variables_map& opts)
{
  return !opts["done"].as<std::string>().empty();
}

const fs::path getDoneFile(const po::variables_map& opts)
{
  return fs::absolute(fs::path(opts["done"].as<std::string>()));
}

const bool hasFileFilter(const po::variables_map& opts)
{
  return !opts["files"].as<std::string>().empty();
}

const boost::regex getFileFilter(const po::variables_map& opts)
{
  const std::string fileRegexStr(opts["files"].as<std::string>());

  return boost::regex(fileRegexStr.empty() ? std::string(".*") : fileRegexStr);
}

const uint getStartupDelay(const po::variables_map& opts)
{
  return opts["pause"].as<uint>();
}

}
}

