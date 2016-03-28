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

namespace options = inshimtu::options;


int main(int argc, char* argv[])
{
  MPICatalystApplication app(&argc, &argv);

  const po::variables_map opts(options::handleOptions(argc, argv));

  // TODO: WatchFilesystem replaces INotify;
  //       WatchFS uses INotify (if all nodes have a node master)
  //       or PollFS (for shared filesystem)
  INotify notify( options::getWatchDirectory(opts)
                , options::getFileFilter(opts)
                , options::getDoneFile(opts));

  std::vector<fs::path> newfiles;

  if (app.isInporter())
  {
    newfiles = options::collectInitialFiles(opts);
  }

  // TODO: Inporter nodes; collect and process files
  std::unique_ptr<Catalyst> catalyst;
  std::unique_ptr<Inporter> inporter;

  if (app.isInporter())
  {
    catalyst.reset(new Catalyst( app.getCommunicator()
                               , options::collectScripts(opts)
                               , options::getStartupDelay(opts)));
    inporter.reset(new Inporter(*catalyst.get()));
  }

  std::cout << "READY" << std::endl;

  do
  {
    // TODO: inporter nodes only
    // TODO: Fix coordination issue where some nodes have a changed file, and some don't
    //       The issue is that collectFiles (via MPI_Allreduce) is waiting for all nodes to report
    if (!app.hasFiles(newfiles))
    {
      // TODO: collect all notifications (MPI lock-step)
      //       sort them by filenaming scheme to determine correct order
      //       (if multiple frames and files created between last processEvents)
      notify.processEvents(newfiles);
    }

    app.collectFiles(newfiles);

    if (app.isInporter())
    {
      inporter->process(newfiles);
    }

    newfiles.clear();

  } while (!app.isDone(notify));

  return 0;
}
