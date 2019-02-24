/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "utils/logger.h"

#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>

#include <cstddef>
#include <vector>
#include <cmath>
#include <iostream>
#include <fstream>

#include <mpi.h>
#include <assert.h>
#include <sys/types.h>


namespace bl = boost::log;


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
  // TODO: Fix logic: logfile can't be open (except maybe by other process?); we're in constructor
  if (logfile.is_open())
  {
    std::cerr << "Error: File already open !" << std::endl;
  }
  else
  {
    int mpiRank = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    char filename[80];
    sprintf(filename, "logs/logfile-rank-%03d.log", mpiRank);
    logfile.open(filename);
  }
}

Logger::~Logger()
{
  logfile.close();
}


void Logger::set_filter(bl::trivial::severity_level level)
{
  bl::core::get()->set_filter
  (
    bl::trivial::severity >= level
  );
}

void Logger::reset_filter()
{
  bl::core::get()->reset_filter();
}


void Logger::write(const char* msg)
{
  logfile << msg << std::endl;
}
