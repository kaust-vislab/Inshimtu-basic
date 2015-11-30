#include "application.h"
#include "logger.h"

#include <iostream>

#include <mpi.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

MPIApplication::MPIApplication(int* argc, char** argv[])
{
  std::cout << "STARTED MPIApplication" << std::endl;

  MPI_Init(argc, argv);

  // create Logger
  Logger::instance();
}

MPIApplication::~MPIApplication()
{
  MPI_Finalize();

  Logger::destroy();

  std::cout << "FINALIZED MPIApplication" << std::endl;
}


