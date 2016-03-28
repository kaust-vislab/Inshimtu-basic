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


Catalyst::Catalyst( vtkMPICommunicatorOpaqueComm& communicator
                  , const std::vector<boost::filesystem::path>& scripts
                  , uint delay)
{
  std::cout << "Starting Catalyst..." << std::endl;

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

void Catalyst::coprocess(vtkDataObject* data,
               double time, uint timeStep, bool forceOutput)
{
  vtkNew<vtkCPDataDescription> dataDescription;

  std::cout << "Catalyst: coprocessing..." << std::endl;

  dataDescription->AddInput("input");
  dataDescription->SetTimeData(time, timeStep);
  if (forceOutput)
  {
    dataDescription->ForceOutputOn();
  }

  if (processor->RequestDataDescription(dataDescription.GetPointer()) != 0)
  {
    // TODO: IMPORTANT! this is where the data should be loaded, calculated (not passed into coprocess)
    dataDescription->GetInputDescriptionByName("input")->SetGrid(data);
    processor->CoProcess(dataDescription.GetPointer());

    std::cout << "RequestDataDescription done." << std::endl;
  }
  else
    std::cerr << "FAILED: RequestDataDescription" << std::endl;

  std::cout << "\t...Done coprocessing." << std::endl;
}

Catalyst::~Catalyst()
{
  processor->Finalize();

  std::cout << "FINALIZED Catalyst." << std::endl;
}
