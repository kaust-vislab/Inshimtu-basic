/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "core/lambda_visitor.hxx"
#include "processing/pipeline.h"
#include "processing/adaptor.h"
#include "utils/logger.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/variant.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>


namespace fs = boost::filesystem;
namespace py = boost::python;

// TODO: use boost.process and bp.system bp.child to process commands
//#include <boost/process.hpp>
//namespace bp = boost::process;

using namespace py;


// TODO: handle uninitialized optional with None
//    https://stackoverflow.com/questions/46327013/exposing-boostoptionalt-via-boost-python-as-internal-reference-or-none
//    https://stackoverflow.com/questions/6274822/boost-python-no-to-python-converter-found-for-stdstring
//    https://stackoverflow.com/questions/36485840/wrap-boostoptional-using-boostpython
template<typename T>
inline T get(boost::optional<T>& o)
{
  return o.get();
}
template<typename T>
inline std::string get_string(T& o)
{
  return o.string();
}


// TODO: remove afer testing...
struct PythonTestStruct
{
  PythonTestStruct() : s("hello world") {}
  //PythonTestStruct(std::string s_) : s(s_) {}
  PythonTestStruct(const std::string& s_) : s(s_) {}
  PythonTestStruct(const boost::filesystem::path& p_) : s(p_.string()) {}

  bool match(std::string s_) { return s == s_; }

  void display() { std::cout << s << std::endl; }

  std::string s;
};


BOOST_PYTHON_MODULE(InshimtuLib)
{
  class_<PythonTestStruct>("PythonTestStruct")
      .def(init<const std::string&>())
      .def(init<boost::filesystem::path&>())
      .def("match", &PythonTestStruct::match)
      .def("display", &PythonTestStruct::display)
  ;

  // Std Boost Utils
  class_<fs::path>("FilesystemPath", init<const std::string&>())
      .def("string", &get_string<fs::path>)
  ;

  class_<boost::regex>("Regex", init<const std::string&>())
      .def("string", &boost::regex::str)
  ;

  class_<boost::optional<fs::path>>("OptionalFilesystemPath", no_init)
      .def("is_initialized", &boost::optional<fs::path>::is_initialized)
      .def("get", &get<fs::path>)
  ;

  class_<ReplaceRegexFormat>("ReplaceRegexFormat", init<boost::regex, std::string>())
  ;


  class_<std::vector<std::string>>("VectorString")
      .def(vector_indexing_suite<std::vector<std::string>>());
  ;

  class_<std::vector<fs::path>>("VectorFilesystemPath")
      .def(vector_indexing_suite<std::vector<fs::path>>());
  ;


  class_<ProcessingSpecCommands::Command>("Command", init<ProcessingSpecCommands::Exe, ProcessingSpecCommands::Args>())
  ;

  class_<std::vector<ProcessingSpecCommands::Command>>("CommandSequence")
      .def(vector_indexing_suite<std::vector<ProcessingSpecCommands::Command>>());
  ;


  // InputSpec
  class_<InputSpec>("InputSpec", init<InputSpecPaths>())
      .def(init<InputSpecPipeline>())
  ;

  class_<InputSpecPaths>("InputSpecPaths", init<const boost::filesystem::path&, const boost::regex&>())
      .def("match", &InputSpecPaths::match)
  ;

  class_<InputSpecPipeline>("InputSpecPipeline")
  ;

  // ProcessingSpec
  class_<ProcessingSpec>("ProcessingSpec", init<ProcessingSpecReadyFile>())
      .def(init<ProcessingSpecCatalyst>())
      .def(init<ProcessingSpecCommands>())
  ;

  class_<ProcessingSpecReadyFile>("ProcessingSpecReadyFile", init<const ReplaceRegexFormat&>())
      .def("get", &ProcessingSpecReadyFile::get)
  ;

  class_<ProcessingSpecCatalyst>("ProcessingSpecCatalyst", init< const std::vector<boost::filesystem::path>&
                                                               , const std::vector<std::string>&>())
      .def("process", &ProcessingSpecCatalyst::process)
  ;

  class_<ProcessingSpecCommands>("ProcessingSpecCommands", init<const std::vector<ProcessingSpecCommands::Command>&>())
      .def("process", &ProcessingSpecCommands::process)
      .def_readonly("FILENAME_ARG",&ProcessingSpecCommands::FILENAME_ARG)
  ;

  // OutputSpec
  class_<OutputSpec>("OutputSpec", init<OutputSpecDone>())
      .def(init<OutputSpecPipeline>())
  ;

  class_<OutputSpecDone>("OutputSpecDone")
  ;

  class_<OutputSpecPipeline>("OutputSpecPipeline")
  ;

  // PipelineSpec
  class_<PipelineSpec>("PipelineSpec", init<InputSpec, ProcessingSpec, OutputSpec>())
  ;

}
