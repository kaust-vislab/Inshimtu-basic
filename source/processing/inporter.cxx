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
                  , const std::vector<std::string>& variables_)
  : section(section_)
  , processor(processor_)
  , availableFiles()
  , variables(variables_)
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
  for(const fs::path& name : newfiles)
  {
    // TODO: verify comparisons are canonical
    //assert(name.is_absolute());

    assert(std::find(std::begin(availableFiles), std::end(availableFiles), name) == std::end(availableFiles));
    assert(std::find(std::begin(workingFiles), std::end(workingFiles), name) == std::end(workingFiles));
    assert(std::find(std::begin(completedFiles), std::end(completedFiles), name) == std::end(completedFiles));

    availableFiles.push_back(name);

    // new file is new
    std::cout << "New available file: '" << name << "'" << std::endl;
  }

  {
    const double time = timeStep * lengthTimeStep;
    const bool forceOutput = false;
    std::vector<std::unique_ptr<TaskState>> tasks;

    // Create inport tasks
    createTasks(time, forceOutput, tasks);

    // TODO: Schedule tasks on inporter nodes
    // Process tasks
    for (auto& task : tasks)
    {
      std::cout << "Processing Task: " << (task->stage.is_initialized() ? task->stage.get().name : "<INVALID>") << std::endl;

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
          std::cout << "Deleting file: '" << name << "'" << std::endl;
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

    std::cout << "\t\t...Done UpdateFields" << std::endl;
  }

/*
  for(const fs::path& name : availableFiles)
  {
    auto afitr = std::find(std::begin(availableFiles), std::end(availableFiles), name);
    auto wfitr = std::find(std::begin(workingFiles), std::end(workingFiles), name);
    auto cfitr = std::find(std::begin(completedFiles), std::end(completedFiles), name);

    if (wfitr == workingFiles.end())
    {
      const double time = timeStep * lengthTimeStep;
      const bool forceOutput = false;
      std::vector<std::unique_ptr<TaskState>> tasks;

      // Create inport tasks
      createTasks(time, name, forceOutput, tasks);

      // new file is new
      std::cout << "New working file: '" << name << "'" << std::endl;

      workingFiles.push_back(name);

      std::cout << "Inporter: Updating data and Catalyst..." << std::endl;

      // TODO: Schedule tasks on inporter nodes
      // Process tasks
      for (auto& task : tasks)
      {
        pipeline_ProcessTask(task);
      }

      // TODO: Fix assumption that processing of file is done
      completedFiles.push_back(name);

      // TODO: Fix - delete should be part of task requests
      // TODO: verify all inporter processing has completed before here
      // TODO: only a single node should delete on shared filesystem, all inporters on local filesystems
      if (deleteFiles && section.getIndex() == MPIInportSection::ROOT_INDEX)
      {
        std::cout << "Deleting file: '" << name << "'" << std::endl;
        fs::remove(name);
      }

      // TODO: Fix assumption that each file represents a timestep
      ++timeStep;

      std::cout << "\t\t...Done UpdateFields" << std::endl;
    }
    else
    {
      std::cerr << "WARNING: input file '" << name << "' "
                << "modified multiple times. "
                << "Expected source files to be written once, and then closed."
                << std::endl;
    }
  }
  */
}

void Inporter::createTasks( double time, bool forceOutput
                          , std::vector<std::unique_ptr<TaskState>>& outTasks)
{
  std::vector<fs::path> accepted;
  std::vector<PipelineSpec> pipelines(this->pipelines);

  // legacy
  {
    // TODO: Processor has scripts (but they apply to all input files).
    std::vector<fs::path> scripts;
    ProcessingSpecCatalyst catalystProcess(scripts, variables);

    PipelineSpec pipeline( "LegacyCatalyst"
                         , InputSpecPipeline()
                         , ProcessingSpecCatalyst(scripts, variables)
                         , OutputSpecDone());

    pipelines.push_back(pipeline);
  }

  for (const auto& pipeline: pipelines)
  {
    accepted.clear();

    if (pipeline_AcceptInput(pipeline, availableFiles, accepted))
    {
      auto mkDescriptor = [&](){ return std::unique_ptr<Descriptor> (new Descriptor(processor, section, timeStep, time, forceOutput)); };

      std::cout << "Pipeline accepted files: " << accepted << std::endl;

      for (const auto& name : accepted)
      {
        auto afitr = std::find(std::begin(availableFiles), std::end(availableFiles), name);
        assert(afitr != std::end(availableFiles));

        availableFiles.erase(afitr);
        workingFiles.push_back(name);
      }

      // TODO: Schedule tasks on inporter nodes
      auto task = pipeline_MkPipelineTask( pipeline, accepted, mkDescriptor);

      outTasks.push_back(std::move(task));
    }
  }
}

