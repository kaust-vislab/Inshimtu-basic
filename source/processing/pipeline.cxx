/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#include "core/lambda_visitor.hxx"
#include "processing/pipeline.h"
#include "processing/adaptorV2.h"
#include "utils/logger.h"

#include "processing/inporters/inporterRawNetCDF.h"
#include "processing/inporters/inporterXMLImage.h"
#include "processing/inporters/inporterXMLPImage.h"
#include "processing/inporters/inporterXMLRectilinear.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>
#include <boost/python.hpp>
#include <boost/process.hpp>
#include <boost/dll.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>


namespace fs = boost::filesystem;
namespace py = boost::python;
namespace bp = boost::process;


namespace
{
bool accept_InputSpecPaths( const InputSpecPaths& ispec
                          , const std::vector<fs::path>& available
                          , std::vector<fs::path>& outAccepted
                          , Attributes& outAttributes)
{
  std::vector<fs::path> filteredAvailable;

  for (const auto& name : available)
  {
    if (ispec.match(name))
    {
      filteredAvailable.push_back(name);

      if (ispec.acceptType == InputSpecPaths::Accept_First)
      {
        break;
      }
    }
  }


  if (ispec.acceptType == InputSpecPaths::Accept_Script)
  {
    if (!Py_IsInitialized())
    {
      Py_Initialize();
    }

    fs::path libPath(boost::dll::program_location().parent_path());
    std::string init_script((boost::format(
        "import os, sys \n"
        "sys.path.insert(0, '%1%') \n"
        "import InshimtuLib as inshimtu \n"
      ) % libPath.string()
      ).str()
    );


    try
    {
      py::object main_module = py::import("__main__");
      py::object main_namespace = main_module.attr("__dict__");
      py::object pyresult;

      pyresult = py::exec( init_script.c_str(), main_namespace);

      main_namespace["ACCEPT_DIRECTORY"] = ispec.directory;
      main_namespace["IN_AVAILABLE"] = filteredAvailable;
      main_namespace["OUT_ACCEPTED"] = py::ptr(&outAccepted);
      main_namespace["OUT_ATTRIBUTES"] = py::ptr(&outAttributes);

      pyresult = py::exec( ispec.acceptScript.c_str(), main_namespace);

      pyresult = py::eval( "accept(IN_AVAILABLE, OUT_ACCEPTED, OUT_ATTRIBUTES)", main_namespace);
      bool result = py::extract<bool>(pyresult);

      return result;
    }
    catch (py::error_already_set const &)
    {
      PyErr_Print();
    }
  }
  else if (!filteredAvailable.empty())
  {
    outAccepted.insert(std::end(outAccepted), std::begin(filteredAvailable), std::end(filteredAvailable));

    return true;
  }

  return false;
};
}


InputSpecAny::InputSpecAny()
  : acceptType(InputSpecPaths::Accept_All)
{
}

void InputSpecAny::setAcceptFirst()
{
  acceptType = InputSpecPaths::Accept_First;
}

void InputSpecAny::setAcceptAll()
{
  acceptType = InputSpecPaths::Accept_All;
}

bool InputSpecAny::operator==(const InputSpecAny& i) const
{
  return acceptType == i.acceptType;
}

bool InputSpecAny::accept( const std::vector<boost::filesystem::path>& available
                              , std::vector<boost::filesystem::path>& outAccepted) const
{
  if (acceptType == InputSpecPaths::Accept_All)
  {
    outAccepted.insert(std::end(outAccepted), std::begin(available), std::end(available));

    return true;
  }
  else if (acceptType == InputSpecPaths::Accept_First && !available.empty())
  {
    outAccepted.push_back(available.front());
    return true;
  }

  return false;
}


ProcessingSpecReadyFile::ProcessingSpecReadyFile(const ReplaceRegexFormat& convert)
  : conversion(convert)
{
}

bool ProcessingSpecReadyFile::operator==(const ProcessingSpecReadyFile& p) const
{
  return conversion == p.conversion;
}

boost::optional<fs::path> ProcessingSpecReadyFile::get(const fs::path& filename) const
{
  const auto path = fs::absolute(filename);

  boost::optional<fs::path> signalledPath;

  std::string signalledFile(boost::regex_replace( path.string()
                                                , conversion.first, conversion.second
                                                , boost::match_default | boost::format_default | boost::format_no_copy));

  if (!signalledFile.empty())
  {
    signalledPath = fs::path(signalledFile);
  }

  return signalledPath;
}


const std::string ProcessingSpecCommands::FILENAME_ARG("$FILENAME");
const std::string ProcessingSpecCommands::FILENAMES_ARRAY_ARG("$FILENAMES_ARRAY");
const std::string ProcessingSpecCommands::TIMESTEP_CODE_ARG("${TIMESTEP_CODE}");

ProcessingSpecCommands::ProcessingSpecCommands(const std::vector<Command>& cmds_) :
  commands(cmds_)
, processCommandsBy(ProcessCommands_All)
, processFilesBy(ProcessFiles_All)
{
}

void ProcessingSpecCommands::setProcessingType(ProcessCommandsType pCmds, ProcessFilesType pFiles)
{
  processCommandsBy = pCmds;
  processFilesBy = pFiles;
}

bool ProcessingSpecCommands::operator==(const ProcessingSpecCommands& p) const
{
  return commands == p.commands
      && processCommandsBy == p.processCommandsBy
      && processFilesBy == p.processFilesBy;
}

bool ProcessingSpecCommands::process( const Attributes& attributes
                                    , const std::vector<fs::path>& files) const
{
  bool result = true;

  if (processFilesBy == ProcessFiles_Single && files.size() > 1)
  {
    if (processCommandsBy == ProcessCommands_Separate)
    {
      for (const auto& cmd: commands)
      {
        for (const auto& filename: files)
        {
          std::vector<fs::path> file({filename});

          bool ret = processCommand(attributes, cmd, file);

          // TODO: enable error handling policy: aggressive (early out), passive (best attempts)
          if (!ret)
          {
            result = false;
          }
        }
      }
      return result;
    }
    else
    {
      for (const auto& filename: files)
      {
        std::vector<fs::path> file({filename});

        bool ret = process(attributes, file);

        // TODO: enable error handling policy: aggressive (early out), passive (best attempts)
        if (!ret)
        {
          result = false;
        }
      }
    }

  }

  for (const auto& cmd: commands)
  {
    bool ret = processCommand(attributes, cmd, files);

    // TODO: enable error handling policy: aggressive (early out), passive (best attempts)
    if (!ret)
    {
      result = false;
    }
  }

  return result;
}

bool ProcessingSpecCommands::processCommand( const Attributes& attributes
                                           , const Command& cmd
                                           , const std::vector<fs::path>& files) const
{
  bool result = true;

  fs::path exePath(cmd.first);

  // bp::system exe mode requires absolute exe path - find from basename and system path
  if (exePath.stem() == exePath.string())
  {
    exePath = bp::search_path(exePath);
  }

  std::string exe = exePath.string();
  std::vector<std::string> args;

  for (const auto& arg: cmd.second)
  {
    if (arg == FILENAME_ARG)
    {
      assert(!files.empty());

      if (files.size() != 1)
      {
        BOOST_LOG_TRIVIAL(warning) << "Argument '" << FILENAME_ARG << "' expects single file (use '"
                                   << FILENAMES_ARRAY_ARG << "' instead). "
                                   << "Found " << files.size() << " files. "
                                   << "Only passing first file to command: " << exe;
      }


      args.push_back(files[0].string());
    }
    else if (arg == FILENAMES_ARRAY_ARG)
    {
      assert(!files.empty());

      for (const auto& file: files)
      {
        args.push_back(file.string());
      }
    }
    else
    {
      std::string targ(arg);

      const auto& otimecode = attributes.getAttribute(TIMESTEP_CODE_ARG);

      if (otimecode.is_initialized())
      {
        if (otimecode->type() == typeid(std::string))
        {
          const auto& value = boost::get<std::string>(otimecode.get());
          boost::replace_all(targ, TIMESTEP_CODE_ARG, value);
        }
        else
        {
          BOOST_LOG_TRIVIAL(warning) << "Attribute '" << TIMESTEP_CODE_ARG << "' has invalid type. "
                                     << "Using attribute name as value for command: "  << exe;
        }
      }

      args.push_back(targ);
    }
  }

  int ret = bp::system(exe, args);

  result = ret == 0;

  return result;
}


ProcessingSpecCatalyst::ProcessingSpecCatalyst(const std::vector<ScriptSpec>& scripts_) :
  scripts(scripts_)
{
}

bool ProcessingSpecCatalyst::operator==(const ProcessingSpecCatalyst& p) const
{
  return scripts == p.scripts;
}

void ProcessingSpecCatalyst::process( const fs::path &filename
                                    , Descriptor& descriptor) const
{
  BOOST_LOG_TRIVIAL(info) << "Processing catalyst spec";

  std::vector<std::unique_ptr<Adaptor>> inporters;
  for (const auto& scriptSpec: scripts)
  {
    const auto& script(scriptSpec.first);
    const auto& vsets(scriptSpec.second);
    // TODO: Pick descriptor (Processor) based on script (because the script was initialized in / processed by the Processor)
    // descriptor = descriptors(script);

    if (RawNetCDFDataFileInporter::canProcess(filename))
    {
      BOOST_LOG_TRIVIAL(info) << "Creating RawNetCDFDataFileInporter for: '" << vsets << "'";

      std::vector<std::string> vars;
      boost::split(vars, vsets, boost::is_any_of(","), boost::token_compress_on);

      for (const auto& v : vars)
      {
        inporters.push_back(
            std::unique_ptr<Adaptor>(
                new RawNetCDFDataFileInporter(descriptor, v)));
      }
    }
    else if (XMLImageDataFileInporter::canProcess(filename))
    {
      BOOST_LOG_TRIVIAL(info) << "Creating XMLImageDataFileInporter for: '" << vsets << "'";

      inporters.push_back(
          std::unique_ptr<Adaptor>(
              new XMLImageDataFileInporter(descriptor, vsets)));
    }
    else if (XMLPImageDataFileInporter::canProcess(filename))
    {
      BOOST_LOG_TRIVIAL(info) << "Creating XMLPImageDataFileInporter for: '" << vsets << "'";

      inporters.push_back(
          std::unique_ptr<Adaptor>(
              new XMLPImageDataFileInporter(descriptor, vsets)));
    }
    else if (XMLRectilinearGridFileInporter::canProcess(filename))
    {
      BOOST_LOG_TRIVIAL(info) << "Creating XMLRectilinearGridFileInporter for: '" << vsets << "'";

      inporters.push_back(
          std::unique_ptr<Adaptor>(
              new XMLRectilinearGridFileInporter(descriptor, vsets)));
    }
    else
    {
         BOOST_LOG_TRIVIAL(error) << "Unable to process the given file, do not have an importer that works.";
    }
  }


  for (auto& inporter : inporters)
  {
    inporter->process(filename);
  }
}


OutputSpecDone::OutputSpecDone()
  : deleteInput(false)
{
}

bool OutputSpecDone::operator==(const OutputSpecDone& o) const
{
  return deleteInput == o.deleteInput;
}


OutputSpecPipeline::OutputSpecPipeline()
  : deleteInput(false)
{
}

bool OutputSpecPipeline::operator==(const OutputSpecPipeline& o) const
{
  return deleteInput == o.deleteInput;
}


PipelineStage::PipelineStage( const std::string& name_
                            , InputSpec input_, ProcessingSpec process_, OutputSpec out_) :
  name(name_)
, input(input_)
, process(process_)
, out(out_)
{
}

bool PipelineStage::operator==(const PipelineStage& stage) const
{
  return name == stage.name
      && input == stage.input
      && process == stage.process
      && out == stage.out;
}


PipelineSpec::PipelineSpec( const std::string& name_
                          , InputSpec input_, ProcessingSpec process_, OutputSpec out_) :
  name(name_)
, stages({PipelineStage(name_, input_, process_, out_)})
{
}

PipelineSpec::PipelineSpec( const PipelineStage& stage_) :
  name(stage_.name)
, stages({stage_})
{
}

PipelineSpec::PipelineSpec( const std::string& name_
                          , const std::vector<PipelineStage>& stages_) :
  name(name_)
, stages(stages_)
{
  assert(stages.size() > 0);
}

const InputSpec PipelineSpec::getInput() const
{
  assert(!stages.empty());

  return stages.front().input;
}


bool Attributes::hasAttribute(const AttributeKey& key) const
{
  const auto& fitr = attributes.find(key);

  return fitr != std::end(attributes);
}

boost::optional<Attributes::AttributeValue> Attributes::getAttribute(const AttributeKey& key) const
{
  boost::optional<AttributeValue> ovalue;

  const auto& fitr = attributes.find(key);

  if (fitr != std::end(attributes))
  {
    ovalue = fitr->second;
  }

  return ovalue;
}

void Attributes::setAttribute(const AttributeKey& key, const AttributeValue& value)
{
  attributes[key] = value;
}


TaskState::TaskState(const PipelineSpec& pipeline_)
  : taskStatus(TaskState::TS_OK)
  , pipeline(pipeline_)
  , stageItr(std::begin(pipeline.stages))
  , inputFiles()
  , outputFiles()
{
}

bool TaskState::canContinue() const
{
  return taskStatus == TS_OK;
}

bool TaskState::wasSuccessful() const
{
  return taskStatus == TS_Done;
}

bool TaskState::hasError() const
{
  return !canContinue() && !wasSuccessful();
}


bool TaskState::hasAttribute(const Attributes::AttributeKey& key) const
{
  return attributes.hasAttribute(key);
}

boost::optional<Attributes::AttributeValue> TaskState::getAttribute(const Attributes::AttributeKey& key) const
{
  return attributes.getAttribute(key);
}

void TaskState::setAttribute(const Attributes::AttributeKey& key, const Attributes::AttributeValue& value)
{
  attributes.setAttribute(key, value);
}

boost::optional<PipelineStage> TaskState::getStage() const
{
  boost::optional<PipelineStage> stage;

  if (stageItr != std::end(pipeline.stages))
  {
    stage = *stageItr;
  }

  return stage;
}

bool TaskState::nextStage()
{
  if (!canContinue())
  {
    return false;
  }

  if (stageItr != std::end(pipeline.stages))
  {
    stageItr++;
  }

  if (stageItr == std::end(pipeline.stages))
  {
    taskStatus = TaskState::TS_Done;
  }

  return canContinue();
}






bool pipeline_AcceptInput( const InputSpec& inputS
                         , const std::vector<fs::path>& available
                         , std::vector<fs::path>& outAccepted
                         , Attributes& outAttributes)
{
  auto visitor = make_lambda_visitor<bool>(
                    [&](const InputSpecPaths& inSp) { return accept_InputSpecPaths( inSp
                                                                                  , available
                                                                                  , outAccepted, outAttributes); }
                  , [&](const InputSpecAny& inSp) { return inSp.accept(available, outAccepted); });

  return boost::apply_visitor(visitor, inputS);
}

std::unique_ptr<TaskState> pipeline_MkPipelineTask( const PipelineSpec& pipeS
                                                  , const std::vector<fs::path>& working
                                                  , const Attributes& attributes
                                                  , std::function<std::unique_ptr<Descriptor>()> mkDescriptor)
{
  std::unique_ptr<TaskState> task(new TaskState(pipeS));

  task->inputFiles.insert(std::end(task->inputFiles), std::begin(working), std::end(working));
  task->attributes = attributes;

  task->mkDescriptor = mkDescriptor;

  return task;
}


void pipeline_ProcessNext(std::unique_ptr<TaskState>& taskS)
{
  if (!taskS->getStage().is_initialized())
    return;
  if (!taskS->canContinue())
    return;

  auto visitorInput = make_lambda_visitor<bool>(
      [&](const InputSpecPaths& inSp)
      {
        if (!taskS->canContinue())
          return false;

        std::vector<fs::path> matchingFiles;

        for (const auto& path: taskS->inputFiles)
        {
          if (inSp.match(path))
            matchingFiles.push_back(path);
        }
        taskS->outputFiles.swap(matchingFiles);

        if (taskS->outputFiles.empty())
          taskS->taskStatus = TaskState::TS_FailedInput;

        return taskS->canContinue();
      }
    , [&](const InputSpecAny&)
        {
          if (!taskS->canContinue())
            return false;

          taskS->outputFiles = taskS->inputFiles;

          return taskS->canContinue();
        });

  auto visitorProcessing = make_lambda_visitor<bool>(
      [&](const ProcessingSpecReadyFile& proSp)
        {
          if (!taskS->canContinue())
            return false;

          std::vector<fs::path> correspondingFiles;

          for (const auto& path: taskS->inputFiles)
          {
            auto oPath = proSp.get(path);
            if (oPath.is_initialized())
              correspondingFiles.push_back(oPath.get());
          }
          taskS->outputFiles.swap(correspondingFiles);
          if (taskS->outputFiles.empty())
            taskS->taskStatus = TaskState::TS_FailedInput;
          return taskS->canContinue();
        }
    , [&](const ProcessingSpecCatalyst& proSp)
        {
          if (!taskS->canContinue())
            return false;
          if (!taskS->mkDescriptor.is_initialized())
          {
            taskS->taskStatus = TaskState::TS_FailedProcessing;
            return false;
          }
          for (const auto& filename: taskS->inputFiles)
          {
            std::unique_ptr<Descriptor> descriptor(taskS->mkDescriptor.get()());
            proSp.process(filename, *descriptor.get());
            // TODO: what are the file products / output? Make inporter->process return created files?
          }
          return taskS->canContinue();
        }
    , [&](const ProcessingSpecCommands& proSp)
        {
          if (!taskS->canContinue())
            return false;

          {
            std::vector<fs::path> producedFiles;
            bool result = proSp.process(taskS->attributes, taskS->inputFiles);

            if (result)
            {
              // TODO: get produced files from process
              //producedFiles.push_back(filename);
            }
            else
            {
              taskS->taskStatus = TaskState::TS_FailedProcessing;
            }
            taskS->outputFiles.swap(producedFiles);
          }

          return taskS->canContinue();
        });

  auto visitorOutput = make_lambda_visitor<bool>(
      [&](const OutputSpecPipeline& outSp)
        {
          if (!taskS->canContinue())
            return false;

          if (outSp.deleteInput)
          {
            // TODO: remove input files
            //       issue: original input files (readyFiles) were lost in previous step
            //       issue: only first inporter (section.first == 0) should remove on shared filesystem
            // fs::remove(path)
          }

          // input readyFiles are output products
          std::swap(taskS->inputFiles, taskS->outputFiles);
          taskS->inputFiles.clear();

          taskS->nextStage();

          return taskS->canContinue();
        }
    , [&](const OutputSpecDone& outSp)
        {
          if (!taskS->canContinue())
            return false;

          if (outSp.deleteInput)
          {
            // TODO: remove input files
            //       issue: original input files (readyFiles) were lost in previous step
            //       issue: only first inporter (section.first == 0) should remove on shared filesystem
            // fs::remove(path)
          }

          // input readyFiles are output products
          std::swap(taskS->inputFiles, taskS->outputFiles);
          taskS->inputFiles.clear();

          taskS->nextStage();

          return taskS->canContinue();
        });

  bool ok = true;

  ok = boost::apply_visitor(visitorInput, taskS->getStage()->input);
  if (ok)
  {
     std::swap(taskS->inputFiles, taskS->outputFiles);
     taskS->outputFiles.clear();
  }

  ok = boost::apply_visitor(visitorProcessing, taskS->getStage()->process);
  if (ok)
  {
     std::swap(taskS->inputFiles, taskS->outputFiles);
     taskS->outputFiles.clear();
  }

  ok = boost::apply_visitor(visitorOutput, taskS->getStage()->out);
  if (ok)
  {
    std::swap(taskS->inputFiles, taskS->outputFiles);
    taskS->outputFiles.clear();
  }
}

void pipeline_ProcessTask(std::unique_ptr<TaskState>& taskS)
{
  if (!taskS->getStage().is_initialized())
    return;

  while (taskS->canContinue())
  {
    pipeline_ProcessNext(taskS);
  }
}

