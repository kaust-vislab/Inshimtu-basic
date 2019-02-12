/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "core/options.h"
#include "core/specifications.h"
#include "core/lambda_visitor.hxx"
#include "utils/help.h"

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


namespace
{
const fs::path getDirectoryPath(const pt::ptree& w_)
{
  std::string watchdirectory(w_.get<std::string>("directory_path", ""));

  return fs::absolute(fs::path(watchdirectory));
}

const boost::regex getFileFilter(const pt::ptree& w_)
{
  std::string fileRegexStr(w_.get<std::string>("files_regex", ""));

  return boost::regex(fileRegexStr.empty() ? std::string(".*") : fileRegexStr);
}

const ProcessingSpecCatalyst::ScriptSpec getScript(const pt::ptree& in_)
{
  fs::path script(in_.get_value<fs::path>("script"));
  std::string vars(in_.get_value<std::string>("variables"));

  return ProcessingSpecCatalyst::ScriptSpec(fs::absolute(script), vars);
}

const std::vector<ProcessingSpecCatalyst::ScriptSpec> getScripts(const pt::ptree& in_)
{
  std::vector<ProcessingSpecCatalyst::ScriptSpec> scripts;

  const auto& scripts_(in_.get_child_optional("scripts"));

  if (scripts_.is_initialized())
  {
    for (const auto& s_: scripts_.get())
    {
      scripts.push_back(getScript(s_.second));
    }
  }

  return scripts;
}

const boost::optional<ReplaceRegexFormat> getReplaceRegexFormat(const pt::ptree& in_)
{
  boost::optional<ReplaceRegexFormat> conversion;

  std::string match_str(in_.get<std::string>("match_regex", ""));
  std::string format_str(in_.get<std::string>("replace_formatstr", ""));

  if (!match_str.empty() && !format_str.empty())
  {
    conversion = ReplaceRegexFormat(boost::regex(match_str), format_str);
  }

  return conversion;
}

const boost::optional<ProcessingSpecCommands::Command> getCommand(const pt::ptree& c_)
{
  boost::optional<ProcessingSpecCommands::Command> command_;

  const auto& cmd_(c_.get<std::string>("cmd", ""));
  if (!cmd_.empty())
  {
    fs::path cmd(cmd_);
    ProcessingSpecCommands::Args args;
    const auto& args_(c_.get_child_optional("args"));

    if (args_.is_initialized())
    {
      for (const auto& a_: args_.get())
      {
        args.push_back(a_.second.get_value<std::string>());
      }
    }
    ProcessingSpecCommands::Command command(ProcessingSpecCommands::Command(fs::absolute(cmd), args));

    command_ = command;
  }

  return command_;
}

typedef std::pair<InputSpecPaths::AcceptType, std::string> AcceptSpec;

const boost::optional<AcceptSpec> getAccept( const pt::ptree& in_
                                           , bool allowScript = true)
{
  boost::optional<AcceptSpec> oaccept;

  const auto& type_(in_.get<std::string>("type", ""));

  if (type_ == "first")
  {
    oaccept = AcceptSpec(InputSpecPaths::Accept_First, "");
  }
  else if (type_ == "all")
  {
    oaccept = AcceptSpec(InputSpecPaths::Accept_All, "");
  }
  else if (type_ == "script" && allowScript)
  {
    std::stringstream ss;
    const auto& scriptlines_(in_.get_child_optional("script"));

    if (scriptlines_.is_initialized())
    {
      for (const auto& sl_: scriptlines_.get())
      {
        ss << sl_.second.get_value<std::string>() << std::endl;
      }
    }

    std::string script(ss.str());

    // TODO: test script for line matching: "$.*def.*accept.*\(.*,.*,.*\).*:.*^"
    assert(!script.empty());
    oaccept = AcceptSpec(InputSpecPaths::Accept_Script, script);
  }
  else
  {
    // TODO: log error
  }

  return oaccept;
}

const boost::optional<InputSpec> getInputSpec( const pt::ptree& in_
                                             , const boost::filesystem::path& libPath)
{
  boost::optional<InputSpec> inputS_;

  const auto& type_(in_.get<std::string>("type", ""));
  if (!type_.empty())
  {
    if (type_ == "InputSpecPaths")
    {
      fs::path dirPath(getDirectoryPath(in_.get_child("watch")));
      boost::regex filesRegex(getFileFilter(in_.get_child("watch")));

      InputSpecPaths inSpec(dirPath, filesRegex);
      const auto& accept_(in_.get_child_optional("accept"));

      if (accept_.is_initialized())
      {
        boost::optional<AcceptSpec> accept(getAccept(accept_.get()));

        if (accept.is_initialized())
        {
          switch (accept->first)
          {
          case InputSpecPaths::Accept_First:
            inSpec.setAcceptFirst();
            break;
          case InputSpecPaths::Accept_All:
            inSpec.setAcceptAll();
            break;
          case InputSpecPaths::Accept_Script:
            inSpec.setAcceptScript(accept->second, libPath);
            break;
          }
        }

      }

      inputS_ = InputSpec(inSpec);
    }
    else if (type_ == "InputSpecAny")
    {
      InputSpecAny inSpec;
      const auto& accept_(in_.get_child_optional("accept"));

      if (accept_.is_initialized())
      {
        boost::optional<AcceptSpec> accept(getAccept(accept_.get(), false));

        if (accept.is_initialized())
        {
          switch (accept->first)
          {
          case InputSpecPaths::Accept_First:
            inSpec.setAcceptFirst();
            break;
          case InputSpecPaths::Accept_All:
            inSpec.setAcceptAll();
            break;
          default:
            assert(false && "Unexpected Accept_Script");
            break;
          }
        }
      }
      else
      {
        // TODO: log error
      }

      inputS_ = InputSpec(inSpec);
    }
  }

  return inputS_;
}

const boost::optional<ProcessingSpec> getProcessSpec( const pt::ptree& in_
                                                 , const boost::filesystem::path& libPath)
{
  boost::optional<ProcessingSpec> processS_;

  const auto& type_(in_.get<std::string>("type", ""));
  if (!type_.empty())
  {
    if (type_ == "ProcessingSpecReadyFile")
    {
      const auto& regex_(getReplaceRegexFormat(in_));

      if (regex_.is_initialized())
      {
        ProcessingSpecReadyFile processSpec(regex_.get());

        processS_ = ProcessingSpec(processSpec);
      }

    }
    else if (type_ == "ProcessingSpecCatalyst")
    {
      std::vector<ProcessingSpecCatalyst::ScriptSpec> scripts(getScripts(in_));

      if (!scripts.empty())
      {
        ProcessingSpecCatalyst processSpec(scripts);

        processS_ = ProcessingSpec(processSpec);
      }
      else
      {
        // TODO: log error
      }
    }
    else if (type_ == "ProcessingSpecCommands")
    {
      std::vector<ProcessingSpecCommands::Command> commands;
      const auto& commands_(in_.get_child_optional("commands"));
      const auto& commandType_(in_.get_child_optional("processingCommandType"));
      const auto& filesType_(in_.get_child_optional("processingFilesType"));

      if (commands_.is_initialized())
      {
        for (const auto& c_: commands_.get())
        {
          boost::optional<ProcessingSpecCommands::Command> command_(getCommand(c_.second));

          if (command_.is_initialized())
          {
            commands.push_back(command_.get());
          }
        }
      }

      if (!commands.empty())
      {
        ProcessingSpecCommands processSpec(commands);

        if (commandType_.is_initialized() && filesType_.is_initialized())
        {
          ProcessingSpecCommands::ProcessCommandsType processCommandsBy(processSpec.processCommandsBy);
          ProcessingSpecCommands::ProcessFilesType processFilesBy(processSpec.processFilesBy);

          if (commandType_->get_value<std::string>() == "all")
          {
            processCommandsBy = ProcessingSpecCommands::ProcessCommands_All;
          }
          else if (commandType_->get_value<std::string>() == "separate")
          {
            processCommandsBy = ProcessingSpecCommands::ProcessCommands_Separate;
          }
          else
          {
            // TODO: log warning
          }

          if (filesType_->get_value<std::string>() == "all")
          {
            processFilesBy = ProcessingSpecCommands::ProcessFiles_All;
          }
          else if (filesType_->get_value<std::string>() == "single")
          {
            processFilesBy = ProcessingSpecCommands::ProcessFiles_Single;
          }
          else
          {
            // TODO: log warning
          }

          processSpec.setProcessingType(processCommandsBy, processFilesBy);
        }

        processS_ = ProcessingSpec(processSpec);
      }
      else
      {
        // TODO: log error
      }
    }
  }

  return processS_;
}

const boost::optional<OutputSpec> getOutputSpec( const pt::ptree& in_
                                               , const boost::filesystem::path& libPath)
{
  boost::optional<OutputSpec> outputS_;

  const auto& type_(in_.get<std::string>("type", ""));
  if (!type_.empty())
  {
    if (type_ == "OutputSpecPipeline")
    {
      OutputSpecPipeline outSpec;

      bool cleanup(in_.get<bool>("deleteInput", false));
      outSpec.deleteInput = cleanup;

      outputS_ = OutputSpec(outSpec);
    }
    else if (type_ == "OutputSpecDone")
    {
      OutputSpecDone outSpec;

      bool cleanup(in_.get<bool>("deleteInput", false));
      outSpec.deleteInput = cleanup;

      outputS_ = OutputSpec(outSpec);
    }
    else
    {
      // TODO: log error
    }
  }

  return outputS_;
}

}

Configuration::Configuration(int argc, const char* const argv[])
  : opts()
  , configs()
  , libPath(fs::canonical(fs::path(argc > 0 ? argv[0] : "./invalid")).parent_path())
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
    ("external_commands,x", po::value<std::vector<std::string>>()->multitoken()->default_value(std::vector<std::string>(), "<none>"), "list of external commands for post processing")
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
    const bool externalCommands(!collectExternalCommands().empty());
    const bool hasPipelines(!collectPipelines().empty());

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

    if (!scriptFiles && !externalCommands && !hasPipelines)
    {
      throw std::runtime_error("the missing option '--scripts' or '--external_commands' or '--config' with pipelines is required");
    }
  }
}

Configuration::~Configuration()
{
}


const std::vector<PipelineSpec> Configuration::collectPipelines() const
{
  std::vector<PipelineSpec> pipelines;

  boost::optional<const pt::ptree&> pipelines_(configs.get_child_optional("pipelines"));

  if (pipelines_.is_initialized())
  {
    for (const auto& p_: pipelines_.get())
    {
      std::vector<PipelineStage> stages;

      const auto& name_(p_.second.get<std::string>("name", "<UNKNOWN>"));

      boost::optional<const pt::ptree&> stages_(p_.second.get_child_optional("stages"));

      if (stages_.is_initialized())
      {
        for (const auto& s_: stages_.get())
        {
          boost::optional<InputSpec> inputSpec(getInputSpec(s_.second.get_child("input"), getLibPath()));
          boost::optional<ProcessingSpec> processSpec(getProcessSpec(s_.second.get_child("process"), getLibPath()));
          boost::optional<OutputSpec> outputSpec(getOutputSpec(s_.second.get_child("output"), getLibPath()));

          if (inputSpec.is_initialized() && processSpec.is_initialized() && outputSpec.is_initialized())
          {
            stages.push_back(PipelineStage(name_, inputSpec.get(), processSpec.get(), outputSpec.get()));
          }
          else
          {
            // TODO: log error
          }
        }
      }

      if (!stages.empty())
      {
        PipelineSpec pipeline( name_, stages);

        pipelines.push_back(pipeline);
      }
      else
      {
        // TODO: log error (pipeline requires at least one stage)
      }
    }
  }

  // legacy
  {
    // TODO: Processor has scripts (but they apply to all input files).
    const auto& vars = collectVariables();
    const auto& scpts = collectScripts();

    if (!scpts.empty())
    {
      std::vector<ProcessingSpecCatalyst::ScriptSpec> scripts;
      for (const auto& vs: vars)
      {
        for (const auto& sc: scpts)
        {
          scripts.push_back(ProcessingSpecCatalyst::ScriptSpec(sc, vs));
        }
      }

      boost::optional<InputSpec> inputS;

      if (hasWatchDirectory() && hasFileFilter())
      {
        inputS = InputSpecPaths(getWatchDirectory(), getFileFilter());
      }
      else
      {
        InputSpecAny inputAny;
        inputAny.setAcceptFirst();

        inputS = inputAny;
      }

      PipelineSpec pipeline( "LegacyCatalyst"
                           , inputS.get()
                           , ProcessingSpecCatalyst(scripts)
                           , OutputSpecDone());

      pipelines.push_back(pipeline);
    }
    else
    {
      // TODO: log error (missing scripts)
    }
  }

  return pipelines;
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

const std::vector<ProcessingSpecCommands::Command> Configuration::collectExternalCommands() const
{
  std::vector<ProcessingSpecCommands::Command> commands;

  {
    boost::optional<const pt::ptree&> commands_(configs.get_child_optional("pipeline.commands"));

    if (commands_.is_initialized())
    {
      for (const auto& c_: commands_.get())
      {
        boost::optional<ProcessingSpecCommands::Command> command_(getCommand(c_.second));

        if (command_.is_initialized())
        {
          commands.push_back(command_.get());
        }
      }
    }
  }

  if (opts.find("external_commands") != opts.end())
  {
    const std::vector<std::string>& ss(opts["external_commands"].as<std::vector<std::string>>());

    if (!ss.empty())
    {
      commands.clear(); // options override configurations
    }

    for (const std::string &sn : ss)
    {
      fs::path cmd(sn);
      ProcessingSpecCommands::Args args({ProcessingSpecCommands::FILENAMES_ARRAY_ARG});
      ProcessingSpecCommands::Command command(ProcessingSpecCommands::Command(fs::absolute(cmd), args));

      commands.push_back(command);
    }
  }

  return commands;
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

const std::vector<Configuration::NodeRange> Configuration::collectInporterNodes() const
{
  // intervals are node id pairs <s,e> specifying the start through end nodes inclusive
  std::vector<NodeRange> nintervals;
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

      nintervals.push_back(NodeRange(n1, n2));
    }
    else
    {
      throw std::runtime_error("the option '--nodes' must have interval format: ((#|(#-#)),)*(#|(#-#))");
    }
  }

  return nintervals;
}


bool Configuration::hasWatchDirectory() const
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

bool Configuration::hasWatchPaths() const
{
  return !getWatchPaths().empty();
}

const std::vector<InputSpecPaths> Configuration::getWatchPaths() const
{
  std::vector<InputSpecPaths> watchPaths;

  const std::vector<PipelineSpec> pipelines(collectPipelines());

  auto visitor = make_lambda_visitor<void>(
                    [&](const InputSpecPaths& inSp) { watchPaths.push_back(inSp); }
                  , [](const InputSpecAny&) { });

  for (const auto& p: pipelines)
  {
    boost::apply_visitor(visitor, p.getInput());
  }

  return watchPaths;
}


bool Configuration::hasOutputReadySignal() const
{
  return getOutputReadyConversion().is_initialized();
}

const boost::optional<ReplaceRegexFormat> Configuration::getOutputReadyConversion() const
{
  boost::optional<ReplaceRegexFormat> conversion;
  boost::optional<const pt::ptree&> nodes_(configs.get_child_optional("input.output_ready_signal"));

  if (nodes_.is_initialized())
  {
    conversion = getReplaceRegexFormat(nodes_.get());
  }

  return conversion;
}



bool Configuration::hasDoneFile() const
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

bool Configuration::hasFileFilter() const
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

uint Configuration::getStartupDelay() const
{
  uint delay(configs.get<uint>("control.initial_connection_wait_secs", 0));

  if (opts.find("pause") != opts.end() && !opts["pause"].defaulted())
  {
    delay = opts["pause"].as<uint>();
  }

  return delay;
}

bool Configuration::getDeleteFilesFlag() const
{
  bool cleanup(configs.get<bool>("control.delete_processed_input_files", false));

  if (opts.find("delete") != opts.end() && !opts["delete"].defaulted())
  {
    cleanup = opts["delete"].as<bool>();
  }

  return cleanup;
}


const fs::path& Configuration::getLibPath() const
{
  return libPath;
}
