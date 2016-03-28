#include "application.h"
#include "logger.h"
#include "notification.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include <mpi.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#include <vtkProcessGroup.h>


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

void MPIApplication::collectFiles(std::vector<fs::path>& newfiles)
{
  char message[2048];
  int total = isRoot() ? 0 : newfiles.size();
  int total_global = 0;

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Allreduce(&total, &total_global, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

  if (isRoot())
  {
    MPI_Status status;

    for (int i = 0; i < total_global; ++i)
    {
      MPI_Recv( message, sizeof(message), MPI_CHAR, MPI_ANY_SOURCE, 10, MPI_COMM_WORLD, &status);
      newfiles.push_back(fs::path(message));
    }
  }
  else
  {
    for (const auto& f : newfiles)
    {
      MPI_Send( f.c_str(), f.string().size()+1, MPI_CHAR, ROOT_RANK, 10, MPI_COMM_WORLD);
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
}

bool MPIApplication::isDone(const INotify& notify)
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
{
  vtkNew<vtkProcessGroup> pgroup;

  // pgroup represents just the inporter processes
  pgroup->Initialize(communicator->GetWorldCommunicator());
  pgroup->RemoveAllProcessIds();
  pgroup->AddProcessId(ROOT_RANK);

  communicator->Initialize(pgroup.Get());
}

MPICatalystApplication::~MPICatalystApplication()
{
}


vtkMPICommunicatorOpaqueComm& MPICatalystApplication::getCommunicator()
{
  return *communicator->GetMPIComm();
}

