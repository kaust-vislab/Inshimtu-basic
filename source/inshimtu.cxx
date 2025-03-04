/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#include "core/options.h"
#include "core/application.h"
#include "utils/logger.h"
#include "sentinels/notification.h"
#include "sentinels/coordinator.h"
#include "processing/adaptorV2.h"
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

  // Replaced INotify with PollFS due to lustre storage system. Could add the 
  // logic back in to use either or if needed.
  //
  //       WatchFS uses INotify (if all nodes have a node master)
  //       or PollFS (for shared filesystem)
  std::unique_ptr<Notify> notify;

  {
    const bool watchPaths(configs.hasWatchPaths());
    const bool doneFile(configs.hasDoneFile());

    if (!watchPaths && !doneFile)
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
                                 , configs));
    inporter.reset(new Inporter( *processor.get()
                               , app.getInporterSection()
                               , configs.collectPipelines()));
  }

  const bool shouldDelete = configs.getDeleteFilesFlag();
  bool isFinished = false;

  BOOST_LOG_TRIVIAL(trace) << "READY";

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

    isFinished = coordinator.update(newfiles, Coordinator::InitialFiles);
    newfiles.clear();
    if (app.isInporter())
    {
      inporter->process(coordinator.getReadyFiles(), false);
    }
  }

  while (!isFinished)
  {
      newfiles.clear();
      notify->processEvents(newfiles);
  
      // Check if polling detected the done file's deletion/modification
      if (notify->isDone()) {
          BOOST_LOG_TRIVIAL(info) << "Exiting: Done file removed or modified.";
          break;
      }
  
      isFinished = coordinator.update(newfiles);
  
      if (app.isInporter())
      {
          inporter->process(coordinator.getReadyFiles(), shouldDelete);
      }
  }

  return 0;
}
