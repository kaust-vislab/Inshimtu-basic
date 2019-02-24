/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#ifndef UTILS_LOGGER_HEADER
#define UTILS_LOGGER_HEADER

#define BOOST_LOG_DYN_LINK 1
#include <boost/log/trivial.hpp>

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

  void set_filter(boost::log::trivial::severity_level level);
  void reset_filter();

  void write(const char* msg);

protected:

  static Logger* singleton;

  std::ofstream logfile;
};


template<typename StreamT, typename T>
StreamT& operator<<(StreamT& os, const std::vector<T>& input)
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
  os << "]";
  return os;
}

template<typename StreamT, typename T>
StreamT& operator<<(StreamT& os, const std::set<T>& input)
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
  os << "]";
  return os;
}


#endif
