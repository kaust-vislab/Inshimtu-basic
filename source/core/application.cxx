/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "core/application.h"
#include "utils/logger.h"

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


MPISection::MPISection( MPISection::NodeRank rank_
                      , SectionIndex index_
                      , SectionSize size_) :
  rank(rank_)
, index(index_)
, size(size_)
{
}

MPIInportSection::MPIInportSection( NodeRank rank
                                  , SectionIndex index
                                  , SectionSize sz) :
  MPISection(rank, index, sz)
{
}


MPIApplication::MPIApplication(int* argc, char** argv[])
{
  MPI_Init(argc, argv);

  // create Logger and set default verbosity
  Logger::instance().set_filter(boost::log::trivial::warning);

  BOOST_LOG_TRIVIAL(trace) << "STARTED MPIApplication";

  int rank;
  int size;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  assert(rank >= 0);
  assert(size > 0);

  appSection.reset(new MPISection(rank, static_cast<size_t>(rank), static_cast<size_t>(size)));

  // TODO: determine node masters; node master watches filesystem
  // (for now verify we are at most 1 MPI process per node?)
}

MPIApplication::~MPIApplication()
{
  MPI_Finalize();

  Logger::destroy();

  BOOST_LOG_TRIVIAL(trace) << "FINALIZED MPIApplication";
}


MPICatalystApplication::MPICatalystApplication(int* argc, char** argv[])
  : MPIApplication(argc, argv)
  , configs(*argc, *argv)
  , notifier(true)
{
  vtkNew<vtkProcessGroup> pgroupInport;
  vtkNew<vtkProcessGroup> pgroupCoord;
  std::set<MPISection::NodeRank> inporters;

  BOOST_LOG_TRIVIAL(trace) << "STARTED MPICatalystApplication";

  int totalSize = static_cast<int>(getSize());

  assert(totalSize > 0);

  // generate inporter node ids from interval pairs
  {
    for (const auto& inval : configs.collectInporterNodes())
    {
      for (int i = inval.first; i < std::min(inval.second+1, totalSize); ++i)
      {
        inporters.insert(i);
      }
    }

    if (inporters.empty())
    {
      for (MPISection::NodeRank i = ROOT_RANK; i < totalSize; ++i)
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

  const static size_t INVALID_INDEX = static_cast<size_t>(-1);
  MPISection::NodeRank inporterRank = -1;
  MPISection::SectionIndex inporterIndex = INVALID_INDEX;
  MPISection::SectionIndex inporterCount = 0;
  for (const MPISection::NodeRank i : inporters)
  {
    pgroupInport->AddProcessId(i);
    if (i != ROOT_RANK)
    {
      pgroupCoord->AddProcessId(i);
    }

    // determine node inport properties
    if (getRank() == i)
    {
      inporterRank = i;
      inporterIndex = inporterCount;
    }
    ++inporterCount;
  }

  assert(inporterRank == getRank() || inporterRank < 0);
  assert(inporterIndex != INVALID_INDEX || inporterRank < 0);
  assert(inporters.size() > 0);

  inporterSection.reset(new MPIInportSection(inporterRank, inporterIndex, inporters.size()));


  inportCommunicator->Initialize(pgroupInport.Get());
  coordCommunicator->Initialize(pgroupCoord.Get());
}

MPICatalystApplication::~MPICatalystApplication()
{
}

const MPIInportSection& MPICatalystApplication::getInporterSection() const
{
  assert(inporterSection.get() != nullptr);

  return *inporterSection;
};


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

