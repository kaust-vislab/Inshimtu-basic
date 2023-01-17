/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#include "core/options.h"
#include "core/application.h"
#include "core/specifications.h"
#include "sentinels/notification.h"
#include "sentinels/coordinator.h"
#include "processing/adaptor.h"
#include "processing/inporter.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <boost/python.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>


namespace po = boost::program_options;
namespace fs = boost::filesystem;
namespace py = boost::python;


int main(int argc, char* argv[])
{
  MPIApplication app(&argc, &argv);

  // process arguments
  if (argc < 3)
  {
    std::cerr << "testComponent expected testname argv[1] and testpath argv[2]" << std::endl;
    return 1;
  }
  fs::path testexe(fs::canonical(fs::path(argv[0])));
  fs::path testexedir(testexe.parent_path());
  std::string testname(argv[1]);
  fs::path testpath(argv[2]);


  // TESTS

  if (testname == "ProcessingSpecCommands")
  {
    ProcessingSpecCommands s({ ProcessingSpecCommands::Command("/usr/bin/echo",{"Reading File: ", ProcessingSpecCommands::FILENAMES_ARRAY_ARG})
                             , ProcessingSpecCommands::Command("/usr/bin/cat",{ProcessingSpecCommands::FILENAMES_ARRAY_ARG})});
    ProcessingSpecCommands ss({ ProcessingSpecCommands::Command("/usr/bin/echo",{"Reading File: ", ProcessingSpecCommands::FILENAME_ARG})
                              , ProcessingSpecCommands::Command("/usr/bin/cat",{ProcessingSpecCommands::FILENAME_ARG})});
    std::vector<fs::path> files;

    files.push_back(testpath / "configs/gdm_outready.json");
    files.push_back(testpath / "configs/gdm_relpath.json");

    bool result(true);
    bool ret;
    Attributes attribs;

    ret = s.process(attribs, files);
    result = result && ret;

    ss.setProcessingType( ProcessingSpecCommands::ProcessCommands_All
                        , ProcessingSpecCommands::ProcessFiles_Single);
    ret = ss.process(attribs, files);
    result = result && ret;

    s.setProcessingType( ProcessingSpecCommands::ProcessCommands_Separate
                       , ProcessingSpecCommands::ProcessFiles_All);
    ret = s.process(attribs, files);
    result = result && ret;

    ss.setProcessingType( ProcessingSpecCommands::ProcessCommands_Separate
                        , ProcessingSpecCommands::ProcessFiles_Single);
    ret = ss.process(attribs, files);
    result = result && ret;

    // TODO: verify stdout and incorporate into result;
    return (result ? 0 : 1);
  }

  if (testname == "ProcessingSpecReadyFile")
  {
    ProcessingSpecReadyFile s(ReplaceRegexFormat(boost::regex("^(.*)/wrfoutReady_(.*)$"), "${1}/wrfout_${2}"));
    bool result(true);

    auto out = s.get(testpath / "data/wrfoutReady_d01_2015-10-30_23:00:00.nc");

    result = out.is_initialized();
    result = *out == testpath / "data/wrfout_d01_2015-10-30_23:00:00.nc" && result;

    return (result ? 0 : 1);
  }

  if (testname == "InputSpecPaths")
  {
    InputSpecPaths s(testpath / "data", boost::regex("wrfoutReady_d01_.*"));
    bool result(true);

    result = s.match(testpath / "data/wrfoutReady_d01_2015-10-30_23:00:00.nc") && result;
    result = !s.match(testpath / "data/wrfout_d01_2015-10-30_23:00:00.nc") && result;

    return (result ? 0 : 1);
  }

  if (testname == "PythonicExpr")
  {
    Py_Initialize();

    bool result(true);

    py::object pyresult = py::eval("5 ** 2");
    int five_squared = py::extract<int>(pyresult);

    result = five_squared == 25;

    return (result ? 0 : 1);
  }

  if (testname == "PythonicExec")
  {
    Py_Initialize();

    bool result(true);

    try
    {
      py::object main_module = py::import("__main__");
      py::object main_namespace = main_module.attr("__dict__");
      py::object pyresult =
          py::exec( "import sys \n"
                    "print('hello world!') \n"
                    "print(dir(sys)) \n"
                    "print(sys.path) \n"
                    "print(sys.__dict__.keys()) \n"
                  , main_namespace);
    }
    catch (py::error_already_set const &)
    {
      PyErr_Print();
      result = false;
    }


    return (result ? 0 : 1);
  }

  if (testname == "PythonicSpec")
  {
    Py_Initialize();

    bool result(true);

    std::string script((boost::format(
        "import os, sys \n"
        "sys.path.insert(0, '%1%') \n"
        "import InshimtuLib as pplz \n"
        "dir(pplz) \n"
        "ispi = pplz.InputSpecAny() \n"
        "dp = pplz.FilesystemPath('%2%/data') \n"
        "r = pplz.Regex('wrfoutReady_d01_.*') \n"
        "isp = pplz.InputSpecPaths(dp, r) \n"
        "fp_match = pplz.FilesystemPath('%2%/data/wrfoutReady_d01_2015-10-30_23:00:00.nc') \n"
        "fp_nomatch = pplz.FilesystemPath('%2%/data/wrfout_d01_2015-10-30_23:00:00.nc') \n"
        "isp.match(fp_match) \n"
        "isp.match(fp_nomatch) \n"
        "ispec = pplz.InputSpec(isp) \n"
        "print(type(ispec)) \n"
        "pr = pplz.Regex('^(.*)/wrfoutReady_(.*)$') \n"
        "rf = pplz.ReplaceRegexFormat(pr, '${1}/wrfout_${2}') \n"
        "psrf = pplz.ProcessingSpecReadyFile(rf) \n"
        "orf = psrf.get(fp_match) \n"
        "print(orf.get().string() if orf.is_initialized() else None) \n"
        "sf = pplz.FilesystemPath('%2%/configs/vti_notified.json') \n"
        "ex_echo = pplz.FilesystemPath('/usr/bin/echo') \n"
        "ex_cat = pplz.FilesystemPath('/usr/bin/cat') \n"
        "args0 = pplz.VectorString() \n"
        "args1 = pplz.VectorString() \n"
        "args0.extend(['Reading File: ', pplz.ProcessingSpecCommands.FILENAME_ARG]) \n"
        "args1.extend([pplz.ProcessingSpecCommands.FILENAME_ARG]) \n"
        "cmd0 = pplz.Command(ex_echo, args0) \n"
        "cmd1 = pplz.Command(ex_cat, args1) \n"
        "cmds = pplz.CommandSequence() \n"
        "cmds.extend([cmd0, cmd1]) \n"
        "psc = pplz.ProcessingSpecCommands(cmds) \n"
        "psc.process(pplz.Attributes(), pplz.VectorFilesystemPath([sf])) \n"
        "cscpts = pplz.VectorFilesystemPath() \n"
        "cscpts.extend([pplz.FilesystemPath(i) for i in ['%2%/pipelines/gridwriter.py','%2%/pipelines/gridviewer_vti_velocity.py']]) \n"
        "cvars = pplz.VectorString() \n"
        "cvars.extend(['U,V,W,QVAPOR']) \n"
        "svspecs = pplz.VectorScriptSpec([pplz.ScriptSpec(i, cvars[0]) for i in cscpts]) \n"
        "pscc = pplz.ProcessingSpecCatalyst(svspecs) \n"
        "pspec = pplz.ProcessingSpec(psc) \n"
        "osp = pplz.OutputSpecDone() \n"
        "ospp = pplz.OutputSpecPipeline() \n"
        "ospec = pplz.OutputSpec(ospp) \n"
        "pipeline = pplz.PipelineSpec('pipeline', ispec, pspec, ospec) \n"
      ) % testexedir.string()
        % testpath.string()
      ).str()
    );

    try
    {
      py::object main_module = py::import("__main__");
      py::object main_namespace = main_module.attr("__dict__");
      py::object pyresult = py::exec( script.c_str(), main_namespace);

    }
    catch (py::error_already_set const &)
    {
      PyErr_Print();
      result = false;
    }

    return (result ? 0 : 1);
  }

  if (testname == "PythonicFile")
  {
    // process arguments
    if (argc < 4)
    {
      std::cerr << "testname='PythonicFile' expected testscript argv[3]" << std::endl;
      return 1;
    }
    fs::path testscript(argv[3]);

    Py_Initialize();

    bool result(true);

    std::string init_script((boost::format(
        "import os, sys \n"
        "sys.path.insert(0, '%1%') \n"
      ) % testexedir.string()
      ).str()
    );

    try
    {
      py::object main_module = py::import("__main__");
      py::object main_namespace = main_module.attr("__dict__");
      main_namespace["TESTPATH"] = testpath.string();
      py::object pyresult;

      pyresult = py::exec( init_script.c_str(), main_namespace);

      pyresult = py::exec_file( (testpath / testscript).c_str()
                              , main_namespace);

    }
    catch (py::error_already_set const &)
    {
      PyErr_Print();
      result = false;
    }

    return (result ? 0 : 1);
  }

  if (testname == "ConfigurationConfigFile")
  {
    // process arguments
    if (argc < 4)
    {
      std::cerr << "testname='ConfigurationConfigFile' expected testconfig argv[3]" << std::endl;
      return 1;
    }
    fs::path testconfig(testpath / argv[3]);

    const char* testconfigCptr(testconfig.c_str());

    const int cfgArgC = 3;
    const char* cfgArgV[cfgArgC] = {argv[0], "-c", testconfigCptr};

    const Configuration configs(cfgArgC, cfgArgV);

    bool result(true);

    const auto pipelines(configs.collectPipelines());

    for (const auto& p: pipelines)
    {
      std::cout << "Pipeline '" << p.name << "' with " << p.stages.size() << " stages" << std::endl;
    }
    std::cout << "Processed (" << pipelines.size() << ") pipelines." << std::endl;

    return (result ? 0 : 1);
  }

  return 1;
}
