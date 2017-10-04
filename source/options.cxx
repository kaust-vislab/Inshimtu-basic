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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

namespace po = boost::program_options;
namespace pt = boost::property_tree;
namespace fs = boost::filesystem;


Configuration::Configuration(int argc, const char* const argv[])
  : opts()
{
  po::options_description basicDesc("basic usage options");
  basicDesc.add_options()
    ("help,h", "help message describing command line options")
    ("version,V", "output version number")
    ("config,c", po::value<std::string>()->default_value(""), "json configuration file")
    ("watch,w", po::value<std::string>()->default_value(""), "pre-existing inporting source directory to watch")
    ("done,d", po::value<std::string>()->default_value(""), "pre-existing termination trigger; done file; file must be outside watch directory")
    ("files,f", po::value<std::string>()->default_value(""), "regular expression of watch directory files to process (ensure expression is 'quoted')")
    ("initial,i", po::value<std::vector<std::string>>()->multitoken()->default_value(std::vector<std::string>(), "<none>"), "space-separated list of pre-exisiting files to process (unquoted for shell expansion)")
    ("scripts,s", po::value<std::vector<std::string>>()->multitoken()->default_value(std::vector<std::string>(), "<none>"), "list of Catalyst scripts for visualization processing")
    ("variables,v", po::value<std::vector<std::string>>()->multitoken()->default_value(std::vector<std::string>(), "<none>"), "space-separated list of comma-separated variable sets to process")
    ("nodes,n", po::value<std::string>()->default_value(""), "comma-separated list of node-id intervals specifying inporter Catalyst nodes")
    ("pause,p", po::value<uint>()->default_value(0), "initial delay in seconds to wait for ParaView to connect before processing commences")
    ("delete", po::bool_switch()->default_value(false), "delete watched files after processing")
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

  // load configuration options
  if (!opts["config"].as<std::string>().empty())
  {
    fs::path cfgs(fs::absolute(fs::path(opts["config"].as<std::string>())));

    try
    {
      pt::read_json(cfgs.string(), configs);
    }
    catch (const pt::ptree_error& e)
    {
      std::stringstream ss;

      ss << "error reading json configuration file: " << cfgs << std::endl;
      ss << "  validate json via `python -m json.tool` command" << std::endl;
      ss << "  try: cat " << cfgs << " | python -m json.tool" << std::endl;
      ss << "ptree_bad_data: " << e.what();

      throw std::runtime_error(ss.str());
    }
  }


  // validate
  {
    const bool watchDirectory(hasWatchDirectory());
    const bool doneFile(hasDoneFile());
    const bool fileFilter(hasFileFilter());
    const bool initialFiles(!collectInitialFiles().empty());
    const bool scriptFiles(!collectScripts().empty());

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

    if (!scriptFiles)
    {
      throw std::runtime_error("the missing option '--scripts' is required");
    }
  }
}

Configuration::~Configuration()
{
}


const std::vector<fs::path> Configuration::collectScripts() const
{
  std::vector<fs::path> scripts;

  boost::optional<const pt::ptree&> scripts_(configs.get_child_optional("pipeline.scripts"));
  if (scripts_.is_initialized())
  {
    for (const auto& s: scripts_.get())
    {
      scripts.push_back(fs::absolute(s.second.get_value<fs::path>()));
    }
  }

  if (opts.find("scripts") != opts.end())
  {
    const std::vector<std::string>& ss(opts["scripts"].as<std::vector<std::string>>());

    if (!ss.empty())
    {
      scripts.clear(); // options override configurations
    }

    for (const std::string &sn : ss)
    {
      fs::path p(sn);
      scripts.push_back(fs::absolute(p));
    }
  }

  return scripts;
}

const std::vector<fs::path> Configuration::collectInitialFiles() const
{
  std::vector<fs::path> files;

  boost::optional<const pt::ptree&> initialfiles_(configs.get_child_optional("input.initial_files"));
  if (initialfiles_.is_initialized())
  {
    for (const auto& i: initialfiles_.get())
    {
      files.push_back(fs::absolute(i.second.get_value<fs::path>()));
    }
  }

  if (opts.find("initial") != opts.end())
  {
    const std::vector<std::string>& ss(opts["initial"].as<std::vector<std::string>>());

    if (!ss.empty())
    {
      files.clear(); // options override configurations
    }

    for (const std::string &sn : ss)
    {
      fs::path p(sn);
      files.push_back(fs::absolute(p));
    }
  }

  return files;
}

const std::vector<std::string> Configuration::collectVariables() const
{
  std::vector<std::string> vars;

  boost::optional<const pt::ptree&> variables_(configs.get_child_optional("pipeline.variables"));
  if (variables_.is_initialized())
  {
    for (const auto& v: variables_.get())
    {
      vars.push_back(v.second.get_value<std::string>());
    }
  }

  if (opts.find("variables") != opts.end())
  {
    const std::vector<std::string>& vs(opts["variables"].as<std::vector<std::string>>());

    if (!vs.empty())
    {
      vars = vs;
    }
  }

  return vars;
}

const std::vector<std::pair<int, int>> Configuration::collectInporterNodes() const
{
  // intervals are node id pairs <s,e> specifying the start through end nodes inclusive
  std::vector<std::pair<int, int>> nintervals;
  std::vector<std::string> ivals;

  boost::optional<const pt::ptree&> nodes_(configs.get_child_optional("control.catalyst_inporter_nodes"));
  if (nodes_.is_initialized())
  {
    for (const auto& n: nodes_.get())
    {
      ivals.push_back(n.second.get_value<std::string>());
    }
  }

  if (opts.find("nodes") != opts.end())
  {
    const std::string ivalstr(opts["nodes"].as<std::string>());
    if (!ivalstr.empty())
    {
      boost::split(ivals, ivalstr, boost::is_any_of(","), boost::token_compress_on);
    }
  }


  // convert into ranges
  const boost::regex regexNums("(\\d+)-(\\d+)|(\\d+)");
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


const bool Configuration::hasWatchDirectory() const
{
  return !configs.get<std::string>("input.watch.directory_path", "").empty()
      || !opts["watch"].as<std::string>().empty();
}

const fs::path Configuration::getWatchDirectory() const
{
  std::string watchdirectory(configs.get<std::string>("input.watch.directory_path", ""));

  std::string optswdir(opts["watch"].as<std::string>());
  if (!optswdir.empty())
  {
    watchdirectory = optswdir;
  }

  return fs::absolute(fs::path(watchdirectory));
}


const bool Configuration::hasOutputReadySignal() const
{
  return getOutputReadyConversion().is_initialized();
}

const boost::optional<Configuration::ReplaceRegexFormat> Configuration::getOutputReadyConversion() const
{
  boost::optional<Configuration::ReplaceRegexFormat> conversion;
  boost::optional<const pt::ptree&> nodes_(configs.get_child_optional("input.output_ready_signal"));

  if (nodes_.is_initialized())
  {
    std::string match_str(nodes_->get<std::string>("match_regex", ""));
    std::string format_str(nodes_->get<std::string>("replace_formatstr", ""));

    if (!match_str.empty() && !format_str.empty())
    {
      conversion = ReplaceRegexFormat(boost::regex(match_str), format_str);
    }
  }

  return conversion;
}



const bool Configuration::hasDoneFile() const
{
  return !configs.get<std::string>("control.done_watchfile", "").empty()
      || !opts["done"].as<std::string>().empty();
}

const fs::path Configuration::getDoneFile() const
{
  std::string watchfileStr(configs.get<std::string>("control.done_watchfile", ""));

  std::string optsfile(opts["done"].as<std::string>());
  if (!optsfile.empty())
  {
    watchfileStr = optsfile;
  }

  return fs::absolute(fs::path(watchfileStr));
}

const bool Configuration::hasFileFilter() const
{
  return !configs.get<std::string>("input.watch.files_regex", "").empty()
      || !opts["files"].as<std::string>().empty();
}

const boost::regex Configuration::getFileFilter() const
{
  std::string fileRegexStr(configs.get<std::string>("input.watch.files_regex", ""));

  std::string optsfileregex(opts["files"].as<std::string>());
  if (!optsfileregex.empty())
  {
    fileRegexStr = optsfileregex;
  }

  return boost::regex(fileRegexStr.empty() ? std::string(".*") : fileRegexStr);
}

const uint Configuration::getStartupDelay() const
{
  uint delay(configs.get<uint>("control.initial_connection_wait_secs", 0));

  if (opts.find("pause") != opts.end() && !opts["pause"].defaulted())
  {
    delay = opts["pause"].as<uint>();
  }

  return delay;
}

const bool Configuration::getDeleteFilesFlag() const
{
  bool cleanup(configs.get<bool>("control.delete_processed_input_files", false));

  if (opts.find("delete") != opts.end() && !opts["delete"].defaulted())
  {
    cleanup = opts["delete"].as<bool>();
  }

  return cleanup;
}

