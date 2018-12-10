#include "logger.h"

#include <cstddef>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

#include <mpi.h>
#include <assert.h>
#include <sys/types.h>

// TODO: use boost.log and make std::cout / std::cerr point to Logger
//#include <boost/logic/trivial.hpp>

Logger* Logger::singleton = nullptr;

Logger& Logger::instance()
{
  if (singleton == nullptr)
  {
    singleton = new Logger();
  }

  return *singleton;
}

void Logger::destroy()
{
  delete singleton;
  singleton = nullptr;
}

Logger::Logger()
{
  if (logfile.is_open())
  {
    std::cout<<"Error: File already open !"<<std::endl;
  }
  else
  {
    int mpiRank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    char filename[80];
    sprintf(filename,"logs/logfile-rank-%03d.log",mpiRank);
    logfile.open(filename);
  }
}

Logger::~Logger()
{
  logfile.close();
}

void Logger::write(const char* msg)
{
  logfile << msg << std::endl;
}
