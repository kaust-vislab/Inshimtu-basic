/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "processing/adaptor.h"
#include "utils/logger.h"

#include <iostream>
#include <math.h>

#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPProcessor.h>
#include <vtkCPPythonScriptPipeline.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPointData.h>


namespace fs = boost::filesystem;

Processor::Processor( vtkMPICommunicatorOpaqueComm& communicator
                    , const std::vector<boost::filesystem::path>& scripts
                    , uint delay)
{
  BOOST_LOG_TRIVIAL(trace) << "Starting Catalyst Processor...";

  if (processor->Initialize(communicator) != 0)
    BOOST_LOG_TRIVIAL(trace) << "Processor initialized with communicator";
  else
    BOOST_LOG_TRIVIAL(error) << "FAILED: Processor initialize";

  for (const fs::path& script : scripts)
  {
    vtkNew<vtkCPPythonScriptPipeline> pipeline;

    if (pipeline->Initialize(script.c_str()) != 0)
      BOOST_LOG_TRIVIAL(trace) << "Pipeline initialized: " << script;
    else
      BOOST_LOG_TRIVIAL(error) << "FAILED: Pipeline initialize: " << script;

    processor->AddPipeline(pipeline.GetPointer());
  }

  BOOST_LOG_TRIVIAL(trace) << "\t\t...Done";
  BOOST_LOG_TRIVIAL(info) << "Please connect in Paraview";
  sleep(delay);
}

Processor::~Processor()
{
  processor->Finalize();

  BOOST_LOG_TRIVIAL(trace) << "FINALIZED Catalyst Processor.";
}


Descriptor::Descriptor( Processor& processor_
                      , const MPIInportSection& section_
                      , uint timeStep, double time, bool forceOutput)
  : processor(processor_)
  , requireProcessing(false)
  , section(section_)
{
  description->SetTimeData(time, timeStep);

  if (forceOutput)
  {
    description->ForceOutputOn();
  }

  requireProcessing = (processor.processor->RequestDataDescription(description.GetPointer()) != 0);
}

Descriptor::~Descriptor()
{
  if (requireProcessing)
  {
    processor.processor->CoProcess(description.GetPointer());
  }
}

bool Descriptor::doesRequireProcessing() const
{
  return requireProcessing;
}


Adaptor::Adaptor( Descriptor& descriptor_
                , const std::string& name_)
  : descriptor(descriptor_)
  , name(name_)
{
  descriptor.description->AddInput(name.c_str());
}

Adaptor::~Adaptor()
{
}

bool Adaptor::doesRequireProcessing() const
{
  return descriptor.doesRequireProcessing();
}

Adaptor::Extent Adaptor::getExtent(size_t max) const
{
  const MPIInportSection& section(descriptor.getSection());

  size_t chunksize = static_cast<size_t>(
                       ceil(static_cast<double>(max) /
                            static_cast<double>(section.getSize())));
  size_t extstart = std::min(chunksize * section.getIndex(), max);
  size_t extsize = std::min(chunksize, max - extstart);

  return Extent(extstart, extsize);
}

const MPIInportSection& Adaptor::getSection() const
{
  return descriptor.getSection();
}


void Adaptor::coprocess(vtkDataObject* data, int global_extent[6])
{
  vtkCPInputDataDescription* inputDescription = descriptor.description->GetInputDescriptionByName(name.c_str());
  assert(inputDescription != nullptr);

  inputDescription->SetGrid(data);
  inputDescription->SetWholeExtent(global_extent);
}
