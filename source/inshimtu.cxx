/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "core/options.h"
#include "core/application.h"
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

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>


namespace po = boost::program_options;
namespace fs = boost::filesystem;


int main(int argc, char* argv[])
{
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

  return 0;
}
