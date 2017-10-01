/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "application.h"
#include "notification.h"
#include "inporter.h"
#include "adaptor.h"
#include "options.h"

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
      notify.reset(new INotify( configs.getWatchDirectory()
                              , configs.getFileFilter()
                              , configs.getDoneFile()));
  }

  std::vector<fs::path> newfiles;

  if (app.isRoot())
  {
    newfiles = configs.collectInitialFiles();
  }

  std::unique_ptr<Processor> processor;
  std::unique_ptr<Inporter> inporter;

  if (app.isInporter())
  {
    processor.reset(new Processor( app.getCommunicator()
                                 , configs.collectScripts()
                                 , configs.getStartupDelay()));
    inporter.reset(new Inporter( *processor.get()
                               , app.getInporterSection()
                               , configs.collectVariables()));
  }

  std::cout << "READY" << std::endl;

  // TODO: Fix logic to support per-node, local files (e.g., from a RAM Disk)
  //       Make each notification node an inporter, each inporter process the file
  //       from its node (the files may have different names), the coordinator node
  //       (root) must syncronize the inporters to process their fragment of the
  //       same frame as other inporters.

  const bool removeProcessedFiles = configs.getDeleteFilesFlag();
  bool deleteFiles = false;

  do
  {
    // if any node has files to process, skip blocking notify event wait
    if (!app.hasFiles(newfiles))
    {
      notify->processEvents(newfiles);
      deleteFiles = removeProcessedFiles;
    }

    app.collectFiles(newfiles);

    if (app.isInporter())
    {
      inporter->process(newfiles, deleteFiles);
    }

    newfiles.clear();

  } while (!app.isDone(*(notify.get())));

  return 0;
}
