/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "core/lambda_visitor.hxx"
#include "processing/pipeline.h"
#include "processing/adaptor.h"
#include "utils/logger.h"

#include "processing/inporters/inporterRawNetCDF.h"
#include "processing/inporters/inporterXMLImage.h"

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

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>


namespace fs = boost::filesystem;
namespace py = boost::python;


// TODO: use boost.process and bp.system bp.child to process commands
//#include <boost/process.hpp>
//namespace bp = boost::process;


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

    std::string init_script((boost::format(
        "import os, sys \n"
        "sys.path.insert(0, '%1%') \n"
        "import InshimtuLib as inshimtu \n"
      ) % ispec.libPath.string()
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

  std::string exe = cmd.first.string();
  std::vector<std::string> args;

  for (const auto& arg: cmd.second)
  {
    if (arg == FILENAME_ARG)
    {
      assert(!files.empty());

      // TODO: log warning (user based error handling policy)
      // assert(files.size() == 1);

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
          // TODO: log warning about invalid TIMESTEP_CODE types
        }
      }

      args.push_back(targ);
    }
  }

  // TODO: use boost.process
  //bp.system(exe, args);

  // legacy
  {
    int ret;
    std::stringstream ss;

    ss << cmd.first;

    for (auto& arg: args)
    {
      // TODO: encode args (replace spaces with "\ ")
      ss << " " << arg;
    }

    ret = system(ss.str().c_str());

    // TODO: enable error handling policy: aggressive (early out), passive (best attempts)
    if (ret < 0)
    {
      // strerror(errno)
      result = false;
    }
    else if (WIFEXITED(ret))
    {
      if (WEXITSTATUS(ret) != 0)
      {
        result = false;
      }
    }
    else
    {
      result = false;
    }
  }

  return result;
}


ProcessingSpecCatalyst::ProcessingSpecCatalyst(const std::vector<ScriptSpec>& scripts_) :
  scripts(scripts_)
{
}

void ProcessingSpecCatalyst::process( const fs::path &filename
                                    , Descriptor& descriptor) const
{
  std::vector<std::unique_ptr<Adaptor>> inporters;

  for (const auto& scriptSpec: scripts)
  {
    const auto& script(scriptSpec.first);
    const auto& vsets(scriptSpec.second);

    // TODO: Pick descriptor (Processor) based on script (because the script was initialized in / processed by the Processor)
    // descriptor = descriptors(script);

    if (RawNetCDFDataFileInporter::canProcess(filename))
    {
      std::cout << "Creating RawNetCDFDataFileInporter for: '" << vsets << "'" << std::endl;

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
      std::cout << "Creating XMLImageDataFileInporter for: '" << vsets << "'" << std::endl;

      inporters.push_back(
          std::unique_ptr<Adaptor>(
              new XMLImageDataFileInporter(descriptor, vsets)));
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

OutputSpecPipeline::OutputSpecPipeline()
  : deleteInput(false)
{
}


PipelineStage::PipelineStage( const std::string& name_
                            , InputSpec input_, ProcessingSpec process_, OutputSpec out_) :
  name(name_)
, input(input_)
, process(process_)
, out(out_)
{
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


// TODO: remove
/*
// Example: traditional
class  input_handler : public boost::static_visitor<void>
{
public:
  void operator()(const InputSpecPaths& inSp) const { std::cout << "InputSpecFile"; }
  void operator()(const InputSpecAny& inSp) const { std::cout << "InputSpecAny"; }
};

void doPipeline(TaskState& pipeSt)
{
  if (pipeSt.stage.is_initialized())
  {
    PipelineSpec& stage = pipeSt.stage.get();
    const InputSpec& inputSp = stage.input;

    boost::apply_visitor(input_handler(), inputSp);
  }
}
*/
