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
#include <boost/python.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>


namespace fs = boost::filesystem;
namespace py = boost::python;


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

void InputSpecPaths::setAcceptScript(const std::string& acceptScript_)
{
  assert(!acceptScript_.empty());

  acceptType = Accept_Script;
  acceptScript = acceptScript_;
}


bool InputSpecPaths::match(const fs::path& filename) const
{
  const auto filepath = fs::absolute(filename);
  return filepath.parent_path() == directory
      && boost::regex_match(filepath.filename().c_str(), filenames);
}


bool InputSpecPaths::accept( const std::vector<fs::path>& available
                           , std::vector<fs::path>& outAccepted) const
{
  // TODO:
  std::vector<fs::path> filteredAvailable;

  for (const auto& name : available)
  {
    if (match(name))
    {
      filteredAvailable.push_back(name);

      if (acceptType == Accept_First)
      {
        break;
      }
    }
  }


  if (acceptType == Accept_Script)
  {
    if (!Py_IsInitialized())
    {
      Py_Initialize();
    }

    // TODO: fix hard coded constant:
    //  fs::path testexe(fs::canonical(fs::path(argv[0])));
    //  fs::path libdir(testexe.parent_path());
    fs::path libdir("/home/holstgr/Development/Inshimtu/build.kvl");

    std::string init_script((boost::format(
        "import os, sys \n"
        "sys.path.insert(0, '%1%') \n"
        "import InshimtuLib as inshimtu \n"
      ) % libdir.string()
      ).str()
    );


    try
    {
      py::object main_module = py::import("__main__");
      py::object main_namespace = main_module.attr("__dict__");
      py::object pyresult;

      pyresult = py::exec( init_script.c_str(), main_namespace);

      main_namespace["ACCEPT_DIRECTORY"] = directory;
      main_namespace["IN_AVAILABLE"] = filteredAvailable;
      main_namespace["OUT_ACCEPTED"] = py::ptr(&outAccepted);

      pyresult = py::exec( acceptScript.c_str(), main_namespace);

      pyresult = py::eval( "accept(IN_AVAILABLE, OUT_ACCEPTED)", main_namespace);
      bool result = py::extract<bool>(pyresult);

      return result;
    }
    catch (py::error_already_set const &)
    {
      PyErr_Print();
    }
  }
  else if (!filteredAvailable.empty())
  {
    outAccepted.insert(std::end(outAccepted), std::begin(filteredAvailable), std::end(filteredAvailable));

    return true;
  }

  return false;
};
