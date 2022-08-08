/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#ifndef CORE_SPECIFICATIONS_HEADER
#define CORE_SPECIFICATIONS_HEADER

#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/variant.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>


// -- types --

typedef std::pair<boost::regex, std::string> ReplaceRegexFormat;


// -- input --

struct InputSpecPaths
{
  enum AcceptType
  {
    Accept_First
  , Accept_All
  , Accept_Script
  };


  InputSpecPaths(const boost::filesystem::path& dir, const boost::regex& filemask);

  void setAcceptFirst();
  void setAcceptAll();
  void setAcceptScript( const std::string& acceptScript_ );


  bool operator==(const InputSpecPaths& inputS) const;

  //! \brief Determine if file can be accepted as input for pipeline
  //! \arg filename   Filepath
  //! \return   True if given filepath matches the directory / filenames specifications
  bool match(const boost::filesystem::path& filename) const;


  boost::filesystem::path directory;
  boost::regex filenames;

  AcceptType acceptType;
  std::string acceptScript;
};


#endif

