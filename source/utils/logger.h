/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#ifndef UTILS_LOGGER_HEADER
#define UTILS_LOGGER_HEADER

#include <iostream>
#include <fstream>
#include <vector>
#include <set>
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

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& input)
{
  bool first = true;
  os << "[";
  for (const auto& i : input)
  {
    if (first)
    {
      first = false;
    }
    else
    {
      os << ",";
    }
    os << i;
  }
  os << "]" << std::flush;
  return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::set<T>& input)
{
  bool first = true;
  os << "[";
  for (const auto& i : input)
  {
    if (first)
    {
      first = false;
    }
    else
    {
      os << ",";
    }
    os << i;
  }
  os << "]" << std::flush;
  return os;
}

#endif
