/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "processing/inporter.h"
#include "processing/adaptor.h"
#include "processing/pipeline.hxx"
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
      Descriptor descriptor(processor, section, timeStep, time, forceOutput);
      std::vector<std::unique_ptr<Adaptor>> inporters;

      // Create inporters
      createInporters(descriptor, name, inporters);

      // new file is new
      std::cout << "New working file: '" << name << "'" << std::endl;

      workingFiles.push_back(name);

      std::cout << "Inporter: Updating data and Catalyst..." << std::endl;

      // Inport file
      for (auto& inporter : inporters)
      {
        inporter->process(name);
      }

      completedFiles.push_back(name);

      // TODO: verify all inporter processing has completed before here
      // TODO: only a single node should delete on shared filesystem, all inporters on local filesystems
      if (deleteFiles && section.getIndex() == MPIInportSection::ROOT_INDEX)
      {
        std::cout << "Deleting file: '" << name << "'" << std::endl;
        fs::remove(name);
      }

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

void Inporter::createInporters( Descriptor& descriptor, const fs::path& filename
                              , std::vector<std::unique_ptr<Adaptor>>& outInporters)
{
  // TODO: Schedule tasks on inporter nodes
  for (const auto& pipeline: pipelines)
  {
    if (pipeline_AcceptInput(pipeline, filename))
    {
      const double time = timeStep * lengthTimeStep;
      const bool forceOutput = false;
      auto mkDescriptor = [&](){ return std::unique_ptr<Descriptor> (new Descriptor(processor, section, timeStep, time, forceOutput)); };

      auto task = pipeline_MkPipelineTask( pipeline, filename, mkDescriptor);

      // TODO: Schedule tasks on inporter nodes
      // outPipelines.push_back(task);
      // TODO: for testing...
      pipeline_ProcessTask(task);
    }
  }

  const bool process_legacy = false;
  if (process_legacy)
  {
    for (const auto& vsets : variables)
    {
      if (RawNetCDFDataFileInporter::canProcess(filename))
      {
        std::cout << "Creating RawNetCDFDataFileInporter for: '" << vsets << "'" << std::endl;

        std::vector<std::string> vars;
        boost::split(vars, vsets, boost::is_any_of(","), boost::token_compress_on);

        for (const auto& v : vars)
        {
          outInporters.push_back(
              std::unique_ptr<Adaptor>(
                  new RawNetCDFDataFileInporter(descriptor, v)));
        }
      }
      else if (XMLImageDataFileInporter::canProcess(filename))
      {
        std::cout << "Creating XMLImageDataFileInporter for: '" << vsets << "'" << std::endl;

        outInporters.push_back(
            std::unique_ptr<Adaptor>(
                new XMLImageDataFileInporter(descriptor, vsets)));
      }
    }
  }
}

// TODO: Untested
/*
         // TODO: don't get the data now, pass in a lamda to delay reading until necessary
        vtkSmartPointer<vtkDataObject> data;

        else if (vtkMPASReader::CanReadFile(name.c_str()))
        {
          data = processMPASDataFile(name);
        }
        else if (vtkNetCDFCFReader::CanReadFile(name.c_str()))
        {
          data = processNetCDFCFDataFile(name);
        }
        else if (vtkNetCDFCFReader::CanReadFile(name.c_str()))
        {
          data = processNetCDFDataFile(name);
        }
*/

