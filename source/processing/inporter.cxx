/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "core/lambda_visitor.hxx"
#include "processing/inporter.h"
#include "processing/adaptor.h"
#include "utils/logger.h"

#include "processing/inporters/inporterRawNetCDF.h"
#include "processing/inporters/inporterXMLImage.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <memory>
#include <iostream>
#include <algorithm>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>


namespace fs = boost::filesystem;

Inporter::Inporter( Processor& processor_
                  , const MPIInportSection& section_
                  , const std::vector<PipelineSpec>& pipelines_)
  : section(section_)
  , pipelines(pipelines_)
  , availableFiles()
  , workingFiles()
  , completedFiles()
  , processor(processor_)
  , timeStep(0)
  , lengthTimeStep(1.0)
{
  std::cout << "STARTED Inporter" << std::endl;
}

Inporter::~Inporter()
{
  std::cout << "FINISHED Inporter. Time:" << timeStep << std::endl;
}

void Inporter::process( const std::vector<fs::path>& newfiles
                      , const bool deleteFiles)
{
  std::cout << "Start Inport Process (" << newfiles.size() << " new files)" << std::endl;

  for(const fs::path& name : newfiles)
  {
    // TODO: verify comparisons are canonical
    //assert(name.is_absolute());

    assert(std::find(std::begin(availableFiles), std::end(availableFiles), name) == std::end(availableFiles));
    assert(std::find(std::begin(workingFiles), std::end(workingFiles), name) == std::end(workingFiles));
    assert(std::find(std::begin(completedFiles), std::end(completedFiles), name) == std::end(completedFiles));

    // TODO: Fix issue with deleteFiles applying to previous unprocessed files
    //       deleteFile status should be associated with the files
    availableFiles.push_back(name);

    // new file is new
    std::cout << "\t\tNew available file: '" << name << "'" << std::endl;
  }


  std::vector<std::unique_ptr<TaskState>> tasks;
  do
  {
    const double time = timeStep * lengthTimeStep;
    const bool forceOutput = false;

    tasks.clear();

    // Create inport tasks
    createTasks(time, forceOutput, tasks);

    // TODO: Schedule tasks on inporter nodes
    // Process tasks
    for (auto& task : tasks)
    {
      std::cout << "\tProcessing Task: " << (task->getStage().is_initialized() ? task->getStage()->name : "<UNKNOWN>")
                << " -- Section index:" << section.getIndex() << " size:" << section.getSize() << " rank:" << section.getRank()
                << std::endl;

      pipeline_ProcessTask(task);

      // TODO: move process accepted files from workingFiles to completeFiles
      // TODO: Fix assumption that processing of file is done
      //completedFiles.push_back(name);
    }

    // TODO: Fix - delete should be part of task requests
    // TODO: verify all inporter processing has completed before here
    // TODO: only a single node should delete on shared filesystem, all inporters on local filesystems
    if (deleteFiles && section.getIndex() == MPIInportSection::ROOT_INDEX)
    {
      for (const auto& name : completedFiles)
      {
        if (std::find(std::begin(workingFiles), std::end(workingFiles), name) != std::end(workingFiles))
        {
          std::cout << "\tDeleting file: '" << name << "'" << std::endl;
          fs::remove(name);
        }
      }
    }

    // TODO: Fix assumptions about timesteps
    if (!tasks.empty())
    {
      ++timeStep;
    }

    // TODO: Fix assumption that processing of file is done
    workingFiles.clear();
  } while (!availableFiles.empty() && !tasks.empty());

  std::cout << "\t\t...Done Inport Process ("
            << availableFiles.size() << " available files, "
            << workingFiles.size() << " working files)" << std::endl;
}

void Inporter::createTasks( double time, bool forceOutput
                          , std::vector<std::unique_ptr<TaskState>>& outTasks)
{
  std::vector<fs::path> accepted;
  Attributes attributes;

  for (const auto& pipeline: pipelines)
  {
    accepted.clear();
    attributes.attributes.clear();

    if (pipeline_AcceptInput( pipeline.getInput()
                            , availableFiles, accepted, attributes))
    {
      auto mkDescriptor = [&,time,forceOutput](){ return std::unique_ptr<Descriptor> (new Descriptor(processor, section, timeStep, time, forceOutput)); };

      std::cout << "\tPipeline accepted files: " << accepted << std::endl;

      for (const auto& name : accepted)
      {
        auto afitr = std::find(std::begin(availableFiles), std::end(availableFiles), name);
        assert(afitr != std::end(availableFiles));

        availableFiles.erase(afitr);
        workingFiles.push_back(name);
      }

      // TODO: Schedule tasks on inporter nodes
      auto task = pipeline_MkPipelineTask( pipeline, accepted, attributes, mkDescriptor);

      outTasks.push_back(std::move(task));
    }
  }
}

