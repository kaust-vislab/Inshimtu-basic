#ifndef LOGGER_HEADER
#define LOGGER_HEADER

#include <iostream>
#include <fstream>
#include <sys/types.h>

class Logger
{
protected:

  Logger();
  ~Logger();

  friend class MPIApplication;
  static void destroy();

public:

  static Logger& instance();

  void write(const char* msg);

protected:

  static Logger* singleton;

  std::ofstream logfile;
};

#endif
