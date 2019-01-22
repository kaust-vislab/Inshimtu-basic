/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
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
  // process arguments
  if (argc < 3)
  {
    std::cerr << "testComponent expected testname argv[1] and testpath argv[2]" << std::endl;
    return 1;
  }
  std::string testname(argv[1]);
  fs::path testpath(argv[2]);


  // TESTS

  if (testname == "ProcessingSpecCommands")
  {
    ProcessingSpecCommands s({ ProcessingSpecCommands::Command("/usr/bin/echo",{"Reading File: ", ProcessingSpecCommands::FILENAME_ARG})
                             , ProcessingSpecCommands::Command("/usr/bin/cat",{ProcessingSpecCommands::FILENAME_ARG})});

    s.process(testpath / "configs/vti_notified.json");

    // TODO: get result from process, verify stdout, and return appropriate result;
    return 0;
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
        "sys.path.insert(0, os.getcwd()) \n"
        "import InshimtuLib as pplz \n"
        "dir(pplz) \n"
        "ispi = pplz.InputSpecPipeline() \n"
        "dp = pplz.FilesystemPath('%1%/data') \n"
        "r = pplz.Regex('wrfoutReady_d01_.*') \n"
        "isp = pplz.InputSpecPaths(dp, r) \n"
        "fp_match = pplz.FilesystemPath('%1%/data/wrfoutReady_d01_2015-10-30_23:00:00.nc') \n"
        "fp_nomatch = pplz.FilesystemPath('%1%/data/wrfout_d01_2015-10-30_23:00:00.nc') \n"
        "isp.match(fp_match) \n"
        "isp.match(fp_nomatch) \n"
        "ispec = pplz.InputSpec(isp) \n"
        "print(type(ispec)) \n"
        "pr = pplz.Regex('^(.*)/wrfoutReady_(.*)$') \n"
        "rf = pplz.ReplaceRegexFormat(pr, '${1}/wrfout_${2}') \n"
        "psrf = pplz.ProcessingSpecReadyFile(rf) \n"
        "orf = psrf.get(fp_match) \n"
        "print(orf.get().string() if orf.is_initialized() else None) \n"
        "sf = pplz.FilesystemPath('%1%/configs/vti_notified.json') \n"
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
        "psc.process(sf) \n"
        "cscpts = pplz.VectorFilesystemPath() \n"
        "cscpts.extend([pplz.FilesystemPath(i) for i in ['%1%/pipelines/gridwriter.py','%1%/pipelines/gridviewer_vti_velocity.py']]) \n"
        "cvars = pplz.VectorString() \n"
        "cvars.extend(['U,V,W,QVAPOR']) \n"
        "pscc = pplz.ProcessingSpecCatalyst(cscpts, cvars) \n"
        "pspec = pplz.ProcessingSpec(psc) \n"
        "osp = pplz.OutputSpecDone() \n"
        "ospp = pplz.OutputSpecPipeline() \n"
        "ospec = pplz.OutputSpec(ospp) \n"
        "pipeline = pplz.PipelineSpec(ispec, pspec, ospec) \n"
      ) % testpath.string()).str()
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

    try
    {
      py::object main_module = py::import("__main__");
      py::object main_namespace = main_module.attr("__dict__");
      main_namespace["TESTPATH"] = testpath.string();
      py::object pyresult;

      pyresult = py::exec( "import os, sys \n"
                           "sys.path.insert(0, os.getcwd()) \n"
                         , main_namespace);

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


  /*
  MPICatalystApplication app(&argc, &argv);
  const Configuration& configs(app.getConfigs());

  // TODO: WatchFilesystem replaces INotify;
  //       WatchFS uses INotify (if all nodes have a node master)
  //       or PollFS (for shared filesystem)
  std::unique_ptr<Notify> notify;

  {
    const bool watchDirectory(configs.hasWatchDirectory());
    const bool doneFile(configs.hasDoneFile());

    if (!watchDirectory && !doneFile)
      notify.reset(new Notify());
    else
      notify.reset(new INotify( configs.getWatchPaths()
                              , configs.getDoneFile()));
  }

  Coordinator coordinator(app, *notify.get(), configs);


  std::unique_ptr<Processor> processor;
  std::unique_ptr<Inporter> inporter;

  if (app.isInporter())
  {
    processor.reset(new Processor( app.getInporterCommunicator()
                                 , configs.collectScripts()
                                 , configs.getStartupDelay()));
    inporter.reset(new Inporter( *processor.get()
                               , app.getInporterSection()
                               , configs.collectVariables()));
  }

  const bool shouldDelete = configs.getDeleteFilesFlag();
  bool isFinished = false;

  std::cout << "READY" << std::endl;

  // TODO: Fix logic to support per-node, local files (e.g., from a RAM Disk)
  //       Make each notification node an inporter, each inporter process the file
  //       from its node (the files may have different names), the coordinator node
  //       (root) must syncronize the inporters to process their fragment of the
  //       same frame as other inporters.

  std::vector<fs::path> newfiles;

  // process initial files (don't delete)
  {
    if (app.isRoot())
    {
      newfiles = configs.collectInitialFiles();
    }

    coordinator.update(newfiles, Coordinator::InitialFiles);
    newfiles.clear();
    if (app.isInporter())
    {
      inporter->process(coordinator.getReadyFiles(), false);
    }
  }

  // process watched files
  do
  {
    notify->processEvents(newfiles);

    isFinished = coordinator.update(newfiles);
    newfiles.clear();

    if (app.isInporter())
    {
      inporter->process(coordinator.getReadyFiles(), shouldDelete);
    }
  } while (!isFinished);
  */

  return 1;
}
