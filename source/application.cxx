#include "application.h"
#include "logger.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <mpi.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#include <vtkProcessGroup.h>
#include <vtkMPI.h>


MPIApplication::MPIApplication(int* argc, char** argv[])
{
  std::cout << "STARTED MPIApplication" << std::endl;

  MPI_Init(argc, argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // TODO: determine node masters; node master watches filesystem
  // (for now verify we are at most 1 MPI process per node?)

  // create Logger
  Logger::instance();
}

MPIApplication::~MPIApplication()
{
  MPI_Finalize();

  Logger::destroy();

  std::cout << "FINALIZED MPIApplication" << std::endl;
}


MPICatalystApplication::MPICatalystApplication(int* argc, char** argv[])
  : MPIApplication(argc, argv)
  , configs(*argc, *argv)
  , notifier(true)
  , inporterSection(-1, 0)
{
  vtkNew<vtkProcessGroup> pgroupInport;
  vtkNew<vtkProcessGroup> pgroupCoord;
  std::set<NodeRank> inporters;

  NodeRank inporterIndex = -1;
  size_t inporterCount = 0;

  // generate inporter node ids from interval pairs
  {
    for (const auto& inval : configs.collectInporterNodes())
    {
      for (int i = inval.first; i < std::min(inval.second+1, getSize()); ++i)
      {
        inporters.insert(i);
      }
    }

    if (inporters.empty())
    {
      for (NodeRank i = ROOT_RANK; i < getSize(); ++i)
      {
        inporters.insert(i);
      }
    }
  }

  // pgroupInport represents just the inporter processes
  pgroupInport->Initialize(inportCommunicator->GetWorldCommunicator());
  pgroupInport->RemoveAllProcessIds();
  // pgroupCoord represents root + inporter processes (ROOT_RANK must be first)
  pgroupCoord->Initialize(coordCommunicator->GetWorldCommunicator());
  pgroupCoord->RemoveAllProcessIds();
  pgroupCoord->AddProcessId(ROOT_RANK);

  for (const NodeRank i : inporters)
  {
    pgroupInport->AddProcessId(i);
    if (i != ROOT_RANK)
    {
      pgroupCoord->AddProcessId(i);
    }

    // determine node inport properties
    if (getRank() == i)
    {
      inporterIndex = inporterCount;
    }
    ++inporterCount;
  }

  inporterSection.first = inporterIndex;
  inporterSection.second = inporterCount;

  inportCommunicator->Initialize(pgroupInport.Get());
  coordCommunicator->Initialize(pgroupCoord.Get());
}

MPICatalystApplication::~MPICatalystApplication()
{
}


vtkMPICommunicatorOpaqueComm& MPICatalystApplication::getInporterCommunicator()
{
  assert(inportCommunicator->GetMPIComm() != nullptr);
  return *inportCommunicator->GetMPIComm();
}

vtkMPICommunicatorOpaqueComm& MPICatalystApplication::getCoordinationCommunicator()
{
  assert(coordCommunicator->GetMPIComm() != nullptr);
  return *coordCommunicator->GetMPIComm();
}

