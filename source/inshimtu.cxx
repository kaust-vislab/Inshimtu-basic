/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "application.h"
#include "notification.h"
#include "inporter.h"
#include "adaptor.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

namespace
{
const std::vector<std::string> collectScripts(int argc, const char* argv[])
{
  std::vector<std::string> scripts;

  for (int i=1; i< argc; ++i)
  {
    scripts.push_back(argv[i]);
  }

  return scripts;
}
}

int main(int argc, char* argv[])
{
  MPIApplication app(&argc, &argv);
  INotify notify;
  Catalyst catalyst(collectScripts(argc, const_cast<const char**>(argv)));
  Inporter inporter(catalyst);

  std::cout << "READY" << std::endl;

  std::vector<std::string> newfiles;
  do
  {
    notify.processEvents(newfiles);
    inporter.process(newfiles);
    newfiles.clear();
  } while (!notify.isDone());

  return 0;
}
