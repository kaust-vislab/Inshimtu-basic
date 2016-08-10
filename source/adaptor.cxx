#include "adaptor.h"
#include "logger.h"

#include <iostream>

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
  std::cout << "Starting Catalyst Processor..." << std::endl;

  processor->Initialize(communicator);

  for (const fs::path& script : scripts)
  {
    vtkNew<vtkCPPythonScriptPipeline> pipeline;

    if (pipeline->Initialize(script.c_str()) != 0)
      std::cout << "Pipeline initialize: " << script << std::endl;
    else
      std::cerr << "FAILED: Pipeline initialize: " << script << std::endl;

    processor->AddPipeline(pipeline.GetPointer());
  }

  std::cout << "\t\t...Done" << std::endl;
  std::cout << "Please connect in Paraview" << std::endl;
  sleep(delay);
}

Processor::~Processor()
{
  processor->Finalize();

  std::cout << "FINALIZED Catalyst Processor." << std::endl;
}


Descriptor::Descriptor( Processor& processor_
                      , uint timeStep, double time, bool forceOutput)
  : processor(processor_)
  , requireProcessing(false)
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

void Adaptor::coprocess(vtkDataObject* data, int global_extent[6])
{
  vtkCPInputDataDescription* inputDescription = descriptor.description->GetInputDescriptionByName(name.c_str());
  assert(inputDescription != nullptr);

  inputDescription->SetGrid(data);
  inputDescription->SetWholeExtent(global_extent);
}
