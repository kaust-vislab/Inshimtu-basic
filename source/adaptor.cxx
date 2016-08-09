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


Adaptor::Adaptor( Processor& processor_
                , const std::vector<std::string>& names_
                , uint timeStep, double time, bool forceOutput)
  : processor(processor_)
  , names(names_)
  , requireProcessing(false)
{
  for (const auto& name : names)
  {
    description->AddInput(name.c_str());
  }

  description->SetTimeData(time, timeStep);

  if (forceOutput)
  {
    description->ForceOutputOn();
  }

  requireProcessing = (processor.processor->RequestDataDescription(description.GetPointer()) != 0);
}

Adaptor::~Adaptor()
{
}

bool Adaptor::doesRequireProcessing() const
{
  return requireProcessing;
}

void Adaptor::setData(vtkDataObject* data, const std::string& name_, int global_extent[6])
{
  vtkCPInputDataDescription* inputDescription = description->GetInputDescriptionByName(name_.c_str());
  assert(inputDescription != nullptr);

  inputDescription->SetGrid(data);
  inputDescription->SetWholeExtent(global_extent);
}

void Adaptor::coprocess()
{
  processor.processor->CoProcess(description.GetPointer());
}
