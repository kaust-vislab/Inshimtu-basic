#include "application.h"
#include "logger.h"
#include "notification.h"
#include "options.h"

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


namespace po = boost::program_options;
namespace fs = boost::filesystem;


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


bool MPIApplication::hasFiles(const std::vector<fs::path>& newfiles) const
{
  int total = newfiles.size();
  int total_global = 0;

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Allreduce(&total, &total_global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  return total_global > 0;
}

void MPIApplication::collectFiles(std::vector<fs::path>& inout_newfiles)
{
  char message[PATH_MAX];
  int total = isRoot() ? 0 : inout_newfiles.size(); // root node doesn't contribute to newfile messages
  int total_global = 0;

  // Note: The following logic assumes: a global filename space, root node will coordinate.

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Allreduce(&total, &total_global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  // root node gathers all new files (including root local)
  if (isRoot())
  {
    MPI_Status status;

    for (int i = 0; i < total_global; ++i)
    {
      MPI_Recv( message, sizeof(message), MPI_CHAR, MPI_ANY_SOURCE, 10, MPI_COMM_WORLD, &status);
      inout_newfiles.push_back(fs::path(message));
    }

    // TODO: sort by filenaming scheme to determine correct order
    //       (if multiple frames and files created between last processEvents)
    std::set<fs::path> unique_newfiles(inout_newfiles.cbegin(), inout_newfiles.cend());
    inout_newfiles.clear();
    inout_newfiles.insert(inout_newfiles.end(), unique_newfiles.cbegin(), unique_newfiles.cend());
  }
  else
  {
    for (const auto& f : inout_newfiles)
    {
      MPI_Send( f.c_str(), f.string().size()+1, MPI_CHAR, ROOT_RANK, 10, MPI_COMM_WORLD);
    }
    inout_newfiles.clear();
  }


  // broadcast newfiles to nodes
  {
    int count;

    if (isRoot())
    {
      count = inout_newfiles.size();
    }

    MPI_Bcast(&count, 1, MPI_INT, ROOT_RANK, MPI_COMM_WORLD);

    for (int i = 0; i < count; ++i)
    {
      if (isRoot())
      {
        const auto& f(inout_newfiles[i]);
        strncpy(message, f.c_str(), std::min(f.string().size()+1, static_cast<size_t>(PATH_MAX)));
      }

      MPI_Bcast(message, sizeof(message), MPI_CHAR, ROOT_RANK, MPI_COMM_WORLD);

      if (!isRoot())
      {
        inout_newfiles.push_back(fs::path(message));
      }
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
}

bool MPIApplication::isDone(const Notify& notify)
{
  const int DONE = 1;
  int done = notify.isDone() ? DONE : 0;
  int done_global = 0;

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Allreduce(&done, &done_global, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  return done_global == DONE;
}


MPICatalystApplication::MPICatalystApplication(int* argc, char** argv[])
  : MPIApplication(argc, argv)
  , configs(*argc, *argv)
  , notifier(true)
  , inporterSection(-1, 0)
{
  vtkNew<vtkProcessGroup> pgroup;
  std::set<int> inporters;

  int inporterIndex = -1;
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
      for (int i = ROOT_RANK; i < getSize(); ++i)
      {
        inporters.insert(i);
      }
    }
  }

  // pgroup represents just the inporter processes
  pgroup->Initialize(communicator->GetWorldCommunicator());
  pgroup->RemoveAllProcessIds();
  for (const int i : inporters)
  {
    pgroup->AddProcessId(i);

    // determine node inport properties
    if (getRank() == i)
    {
      inporterIndex = inporterCount;
    }
    ++inporterCount;
  }

  inporterSection.first = inporterIndex;
  inporterSection.second = inporterCount;

  communicator->Initialize(pgroup.Get());
}

MPICatalystApplication::~MPICatalystApplication()
{
}


vtkMPICommunicatorOpaqueComm& MPICatalystApplication::getCommunicator()
{
  return *communicator->GetMPIComm();
}

