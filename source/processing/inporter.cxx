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
  : processor(processor_)
  , section(section_)
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
    auto wfitr = std::find(std::begin(workingFiles), std::end(workingFiles), name);
    auto cfitr = std::find(std::begin(completedFiles), std::end(completedFiles), name);
    if (cfitr != completedFiles.end())
    {
      // TODO: Verify: is this event ever expected?  Duplicates functionality from Coordinator?

      // expected event
      std::cout << "Completed file: '" << name << "'" << std::endl;

      completedFiles.erase(cfitr);

      assert(wfitr != workingFiles.end() && "Expected file in working set");
    }
    else if (wfitr == workingFiles.end())
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
}

void Inporter::createTasks( double time, const boost::filesystem::path& filename, bool forceOutput
                          , std::vector<std::unique_ptr<TaskState>>& outTasks)
{
  std::vector<PipelineSpec> pipelines(this->pipelines);

  // legacy
  {
    std::vector<fs::path> scripts; // TODO: Processor has scripts (but they apply to all input files).
    ProcessingSpecCatalyst catalystProcess(scripts, variables);

    PipelineSpec pipeline( InputSpecPipeline()
                         , ProcessingSpecCatalyst(scripts, variables)
                         , OutputSpecDone());

    pipelines.push_back(pipeline);
  }

  for (const auto& pipeline: pipelines)
  {
    if (pipeline_AcceptInput(pipeline, filename))
    {
      auto mkDescriptor = [&](){ return std::unique_ptr<Descriptor> (new Descriptor(processor, section, timeStep, time, forceOutput)); };

      // TODO: Schedule tasks on inporter nodes
      auto task = pipeline_MkPipelineTask( pipeline, filename, mkDescriptor);

      outTasks.push_back(std::move(task));
    }
  }
}

