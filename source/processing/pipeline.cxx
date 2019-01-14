/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "processing/pipeline.hxx"
#include "processing/inporter.h"
#include "processing/adaptor.h"
#include "utils/logger.h"

#include <iostream>
#include <vector>
#include <string>

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


OutputSpecDone::OutputSpecDone()
  : deleteInput(false)
{
}

OutputSpecPipeline::OutputSpecPipeline()
  : deleteInput(false)
  , pipeline(nullptr)
{
}



TaskState::TaskState()
  : taskStatus(TaskState::TS_OK)
  , stage()
  , readyFiles()
  , products()
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


bool pipeline_AcceptInput(const PipelineSpec& pipeS, const boost::filesystem::path& filename)
{
  auto visitor = make_lambda_visitor<bool>(
                    [&](const InputSpecPaths& inSp) { return inSp.match(filename); }
                  , [](const InputSpecPipeline&) { return true; });

  return boost::apply_visitor(visitor, pipeS.input);
}

std::unique_ptr<TaskState> pipeline_MkPipelineTask( const PipelineSpec& pipeS
                                                  , const boost::filesystem::path& filename
                                                  , std::function<std::unique_ptr<Descriptor>()> mkDescriptor)
{
  std::unique_ptr<TaskState> task;

  task->stage = pipeS;
  task->readyFiles.push_back(filename);

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

                      std::vector<boost::filesystem::path> matchingFiles;

                      for (const auto& path: taskS->readyFiles)
                      {
                        if (inSp.match(path))
                          matchingFiles.push_back(path);
                      }
                      taskS->products.swap(matchingFiles);

                      if (taskS->products.empty())
                        taskS->taskStatus = TaskState::TS_FailedInput;

                      return taskS->canContinue();
                    }
                  , [&](const InputSpecPipeline&)
                      {
                        if (!taskS->canContinue())
                          return false;

                        taskS->products = taskS->readyFiles;

                        return taskS->canContinue();
                      });
  auto visitorProcessing = make_lambda_visitor<bool>(
                    [&](const ProcessingSpecReadyFile& proSp)
                      {
                        if (!taskS->canContinue())
                          return false;

                        std::vector<boost::filesystem::path> correspondingFiles;

                        for (const auto& path: taskS->readyFiles)
                        {
                          auto oPath = proSp.get(path);
                          if (oPath.is_initialized())
                            correspondingFiles.push_back(oPath.get());
                        }
                        taskS->products.swap(correspondingFiles);

                        if (taskS->products.empty())
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

                        for (const auto& filename: taskS->readyFiles)
                        {
                          std::unique_ptr<Descriptor> descriptor(taskS->mkDescriptor.get()());
                          std::vector<std::unique_ptr<Adaptor>> inporters;

                          for (const auto& vsets : proSp.variables)
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
                                        new RawNetCDFDataFileInporter(*descriptor.get(), v)));
                              }
                            }
                            else if (XMLImageDataFileInporter::canProcess(filename))
                            {
                              std::cout << "Creating XMLImageDataFileInporter for: '" << vsets << "'" << std::endl;

                              inporters.push_back(
                                  std::unique_ptr<Adaptor>(
                                      new XMLImageDataFileInporter(*descriptor.get(), vsets)));
                            }
                          }

                          for (auto& inporter : inporters)
                          {
                            inporter->process(filename);
                          }

                          // TODO: what are the file products / output? Make inporter->process return created files?
                        }

                        return taskS->canContinue();
                      }
                  , [&](const ProcessingSpecCommands& proSp)
                      {
                        if (!taskS->canContinue())
                          return false;

                        // TODO: Implement Command Processing
                        taskS->taskStatus = TaskState::TS_FailedProcessing;

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
                        std::swap(taskS->readyFiles, taskS->products);
                        taskS->readyFiles.clear();

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
                        std::swap(taskS->readyFiles, taskS->products);
                        taskS->readyFiles.clear();

                        taskS->stage.reset();
                        taskS->taskStatus = TaskState::TS_Done;

                        return taskS->canContinue();
                      });

  bool ok = true;

  ok = boost::apply_visitor(visitorInput, taskS->stage->input);
  if (ok)
  {
     std::swap(taskS->readyFiles, taskS->products);
     taskS->products.clear();
  }

  ok = boost::apply_visitor(visitorProcessing, taskS->stage->process);
  if (ok)
  {
     std::swap(taskS->readyFiles, taskS->products);
     taskS->products.clear();
  }

  ok = boost::apply_visitor(visitorOutput, taskS->stage->out);
  if (ok)
  {
    std::swap(taskS->readyFiles, taskS->products);
    taskS->products.clear();
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
  void operator()(const InputSpecPipeline& inSp) const { std::cout << "InputSpecPipeline"; }
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
