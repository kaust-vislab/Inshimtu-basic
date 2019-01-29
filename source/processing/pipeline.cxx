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
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>


namespace fs = boost::filesystem;

// TODO: use boost.process and bp.system bp.child to process commands
//#include <boost/process.hpp>
//namespace bp = boost::process;


bool InputSpecAny::accept( const std::vector<boost::filesystem::path>& available
                              , std::vector<boost::filesystem::path>& outAccepted) const
{
  outAccepted.insert(std::end(outAccepted), std::begin(available), std::end(available));
  return true;
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

bool ProcessingSpecCommands::process(const std::vector<fs::path>& files) const
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

          bool ret = processCommand(cmd, file);

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

        bool ret = process(file);

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
    bool ret = processCommand(cmd, files);

    // TODO: enable error handling policy: aggressive (early out), passive (best attempts)
    if (!ret)
    {
      result = false;
    }
  }

  return result;
}

bool ProcessingSpecCommands::processCommand( const Command& cmd
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
      args.push_back(arg);
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


ProcessingSpecCatalyst::ProcessingSpecCatalyst( const std::vector<fs::path>& scripts_
                                              , const std::vector<std::string>& variables_) :
  scripts(scripts_)
, variables(variables_)
{
}

void ProcessingSpecCatalyst::process( const fs::path &filename
                                    , Descriptor& descriptor) const
{
  std::vector<std::unique_ptr<Adaptor>> inporters;

  for (const auto& vsets : variables)
  {
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
  , pipeline(nullptr)
{
}


PipelineSpec::PipelineSpec( const std::string& name_
                          , InputSpec input_, ProcessingSpec process_, OutputSpec out_) :
  name(name_)
, input(input_)
, process(process_)
, out(out_)
{
}


TaskState::TaskState()
  : taskStatus(TaskState::TS_OK)
  , stage()
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


bool pipeline_AcceptInput( const PipelineSpec& pipeS
                         , const std::vector<fs::path>& available
                         , std::vector<fs::path>& outAccepted)
{
  auto visitor = make_lambda_visitor<bool>(
                    [&](const InputSpecPaths& inSp) { return inSp.accept(available, outAccepted); }
                  , [&](const InputSpecAny& inSp) { return inSp.accept(available, outAccepted); });

  return boost::apply_visitor(visitor, pipeS.input);
}

std::unique_ptr<TaskState> pipeline_MkPipelineTask( const PipelineSpec& pipeS
                                                  , const std::vector<fs::path>& working
                                                  , std::function<std::unique_ptr<Descriptor>()> mkDescriptor)
{
  std::unique_ptr<TaskState> task(new TaskState());

  task->stage = pipeS;
  task->inputFiles.insert(std::end(task->inputFiles), std::begin(working), std::end(working));

  task->mkDescriptor = mkDescriptor;

  return task;
}


void pipeline_ProcessNext(std::unique_ptr<TaskState>& taskS)
{
  if (!taskS->stage.is_initialized())
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

          // TODO: Implement Catalyst Processing

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
            bool result = proSp.process(taskS->inputFiles);

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

          taskS->stage.reset();
          if (outSp.pipeline != nullptr)
          {
            taskS->stage = *outSp.pipeline;
          }
          else
          {
            taskS->taskStatus = TaskState::TS_Done;
          }

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

          taskS->stage.reset();
          taskS->taskStatus = TaskState::TS_Done;

          return taskS->canContinue();
        });

  bool ok = true;

  ok = boost::apply_visitor(visitorInput, taskS->stage->input);
  if (ok)
  {
     std::swap(taskS->inputFiles, taskS->outputFiles);
     taskS->outputFiles.clear();
  }

  ok = boost::apply_visitor(visitorProcessing, taskS->stage->process);
  if (ok)
  {
     std::swap(taskS->inputFiles, taskS->outputFiles);
     taskS->outputFiles.clear();
  }

  ok = boost::apply_visitor(visitorOutput, taskS->stage->out);
  if (ok)
  {
    std::swap(taskS->inputFiles, taskS->outputFiles);
    taskS->outputFiles.clear();
  }
}

void pipeline_ProcessTask(std::unique_ptr<TaskState>& taskS)
{
  if (!taskS->stage.is_initialized())
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
