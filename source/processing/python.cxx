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
#include <boost/python/stl_iterator.hpp>
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
inline T get_optional(boost::optional<T>& o)
{
  return o.get();
}
template<typename T>
inline T get_shared(boost::shared_ptr<T>& o)
{
  return *o.get();
}
template<typename T>
inline std::string get_string(T& o)
{
  return o.string();
}

template<typename S, typename T>
inline T convert(const S& o)
{
  return T(o);
}

// https://stackoverflow.com/questions/4819707/passing-python-list-to-c-vector-using-boost-python
// https://stackoverflow.com/questions/18793952/boost-python-how-do-i-provide-a-custom-constructor-wrapper-function
template<typename T>
boost::shared_ptr<std::vector<T>> pylist_to_vector(py::list o)
{
  std::vector<T> v;
  py::stl_input_iterator<T> begin(o);
  py::stl_input_iterator<T> end;

  v.insert(v.end(), begin, end);
  return boost::shared_ptr<std::vector<T>>(new std::vector<T>(v));
}


// TODO: fix / remove hacks
///////////////////////////////////////////////
/// TODO: Fix this abomination when we have a better way to pass all required runtime environments to tasks
std::unique_ptr<TaskState> pipeline_MkPipelineTask_NoCatalyst( const PipelineSpec& pipeS
                                                  , const std::vector<fs::path>& working)
{
  std::unique_ptr<TaskState> task(new TaskState());

  task->stage = pipeS;
  task->inputFiles.insert(std::end(task->inputFiles), std::begin(working), std::end(working));

  return task;
}

boost::shared_ptr<TaskState> pipeline_MkPipelineTask_NoCatalyst_ptr( const PipelineSpec& pipeS
                                                 , const std::vector<boost::filesystem::path>& working)
{
  std::unique_ptr<TaskState> ptr(pipeline_MkPipelineTask_NoCatalyst(pipeS, working));

  return boost::shared_ptr<TaskState>(ptr.release());
}
void pipeline_ProcessTask_ptr(boost::shared_ptr<TaskState>& taskS)
{
  std::unique_ptr<TaskState> ptr(taskS.get());

  pipeline_ProcessTask(ptr);

  ptr.release();
}
///////////////////////////////////////////////



BOOST_PYTHON_MODULE(InshimtuLib)
{
  // Std Boost Utils
  class_<fs::path>("FilesystemPath", init<const std::string&>())
      .def("string", &get_string<fs::path>)
  ;

  class_<boost::regex>("Regex", init<const std::string&>())
      .def("string", &boost::regex::str)
  ;

  class_<boost::optional<fs::path>>("OptionalFilesystemPath", no_init)
      .def("is_initialized", &boost::optional<fs::path>::is_initialized)
      .def("get", &get_optional<fs::path>)
  ;

  class_<ReplaceRegexFormat>("ReplaceRegexFormat", init<boost::regex, std::string>())
  ;


  class_<std::vector<std::string>>("VectorString")
      .def(init<size_t>())
      .def(init<std::vector<std::string>>())
      .def("__init__", make_constructor(&pylist_to_vector<std::string>))
      .def(vector_indexing_suite<std::vector<std::string>>())
  ;

  class_<std::vector<fs::path>>("VectorFilesystemPath")
      .def(init<size_t>())
      .def(init<std::vector<fs::path>>())
      .def("__init__", make_constructor(&pylist_to_vector<fs::path>))
      .def(vector_indexing_suite<std::vector<fs::path>>())
  ;

  def("CommandExe", &convert<std::string, fs::path>);

  class_<ProcessingSpecCommands::Command>("Command", init<ProcessingSpecCommands::Exe, ProcessingSpecCommands::Args>())
  ;

  class_<std::vector<ProcessingSpecCommands::Command>>("CommandSequence")
      .def(init<size_t>())
      .def(init<std::vector<ProcessingSpecCommands::Command>>())
      .def("__init__", make_constructor(&pylist_to_vector<ProcessingSpecCommands::Command>))
      .def(vector_indexing_suite<std::vector<ProcessingSpecCommands::Command>>())
  ;


  // InputSpec
  class_<InputSpec>("InputSpec", init<InputSpecPaths>())
      .def(init<InputSpecAny>())
  ;

  {
    scope scope =
    class_<InputSpecPaths>("InputSpecPaths", init<const boost::filesystem::path&, const boost::regex&>())
        .def("setAcceptFirst", &InputSpecPaths::setAcceptFirst)
        .def("setAcceptAll", &InputSpecPaths::setAcceptAll)
        .def("setAcceptScript", &InputSpecPaths::setAcceptScript)
        .def("match", &InputSpecPaths::match)
        .def("accept", &InputSpecPaths::accept)
    ;

    enum_<InputSpecPaths::AcceptType>("AcceptType")
      .value("ACCEPT_FIRST", InputSpecPaths::Accept_First)
      .value("ACCEPT_ALL", InputSpecPaths::Accept_All)
      .value("ACCEPT_ALL", InputSpecPaths::Accept_Script)
      .export_values()
    ;
  }

  class_<InputSpecAny>("InputSpecAny")
      .def("accept", &InputSpecAny::accept)
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


  {
    scope scope =
    class_<ProcessingSpecCommands>("ProcessingSpecCommands", init<const std::vector<ProcessingSpecCommands::Command>&>())
        .def("process", &ProcessingSpecCommands::process)
        .def("setProcessingType", &ProcessingSpecCommands::setProcessingType)
        .def_readonly("FILENAME_ARG",&ProcessingSpecCommands::FILENAME_ARG)
        .def_readonly("FILENAMES_ARRAY_ARG",&ProcessingSpecCommands::FILENAMES_ARRAY_ARG)
    ;

    enum_<ProcessingSpecCommands::ProcessCommandsType>("ProcessCommandsType")
      .value("ProcessCommands_All", ProcessingSpecCommands::ProcessCommands_All)
      .value("ProcessCommands_Separate", ProcessingSpecCommands::ProcessCommands_Separate)
      .export_values()
    ;

    enum_<ProcessingSpecCommands::ProcessFilesType>("ProcessFilesType")
      .value("ProcessFiles_All", ProcessingSpecCommands::ProcessFiles_All)
      .value("ProcessFiles_Single", ProcessingSpecCommands::ProcessFiles_Single)
      .export_values()
    ;
  }

  // OutputSpec
  class_<OutputSpec>("OutputSpec", init<OutputSpecDone>())
      .def(init<OutputSpecPipeline>())
  ;

  class_<OutputSpecDone>("OutputSpecDone")
  ;

  class_<OutputSpecPipeline>("OutputSpecPipeline")
  ;

  // PipelineSpec
  class_<PipelineSpec>("PipelineSpec", init<std::string, InputSpec, ProcessingSpec, OutputSpec>())
      .def_readonly("name",&PipelineSpec::name)
  ;

  class_<boost::optional<PipelineSpec>>("OptionalPipelineSpec", no_init)
      .def("is_initialized", &boost::optional<PipelineSpec>::is_initialized)
      .def("get", &get_optional<PipelineSpec>)
  ;


  // PipelineProcessing
  {
    scope scope =
    class_<TaskState>("TaskState", no_init)
        .def("canContinue", &TaskState::canContinue)
        .def("wasSuccessful", &TaskState::wasSuccessful)
        .def("hasError", &TaskState::hasError)
        .def_readonly("taskStatus", &TaskState::taskStatus)
        .def_readonly("inputFiles", &TaskState::inputFiles)
        .def_readonly("outputFiles", &TaskState::outputFiles)
    ;

    enum_<TaskState::TaskStatus>("TaskStatus")
      .value("TS_OK", TaskState::TS_OK)
      .value("TS_DONE", TaskState::TS_Done)
      .value("TS_ERROR", TaskState::TS_Error)
      .value("TS_FAILED_INPUT", TaskState::TS_FailedInput)
      .value("TS_FAILED_PROCESSING", TaskState::TS_FailedProcessing)
      .export_values()
    ;
  }

  class_<boost::shared_ptr<TaskState>>("PtrTaskState", no_init)
      .def("get", &get_shared<TaskState>)
  ;


  def("pipelineAcceptInput", &pipeline_AcceptInput);

  // TODO: restore when all argument types supported
  //def("pipelineMkPipelineTask", &pipeline_MkPipelineTask);
  def("pipelineMkPipelineTaskNoCatalyst", &pipeline_MkPipelineTask_NoCatalyst_ptr);

  def("pipelineProcessTask", &pipeline_ProcessTask_ptr);
}
