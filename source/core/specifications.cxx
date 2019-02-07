/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "core/specifications.h"
#include "utils/logger.h"

#include <iostream>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/variant.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/format.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>


namespace fs = boost::filesystem;


InputSpecPaths::InputSpecPaths(const fs::path& dir
                              , const boost::regex& filemask)
  : directory(fs::absolute(dir))
  , filenames(filemask)
  , acceptType(Accept_First)
{
}

void InputSpecPaths::setAcceptFirst()
{
  acceptType = Accept_First;
  acceptScript.clear();
}

void InputSpecPaths::setAcceptAll()
{
  acceptType = Accept_All;
  acceptScript.clear();
}

void InputSpecPaths::setAcceptScript( const std::string& acceptScript_
                                    , const boost::filesystem::path& libPath_)
{
  assert(!acceptScript_.empty());
  assert(!libPath_.empty());
  assert(fs::is_directory(libPath_));

  acceptType = Accept_Script;
  acceptScript = acceptScript_;
  libPath = libPath_;
}


bool InputSpecPaths::match(const fs::path& filename) const
{
  const auto filepath = fs::absolute(filename);
  return filepath.parent_path() == directory
      && boost::regex_match(filepath.filename().c_str(), filenames);
}

