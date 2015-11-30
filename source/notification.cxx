#include "notification.h"
#include "logger.h"

#include <iostream>
#include <algorithm>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

INotify::INotify()
  : inotify_descriptor(-1)
  , done_descriptor(-1)
  , watch_descriptor(-1)
{
  std::cout << "STARTED INotify" << std::endl;

  // TODO: Improve
  // srcspath must exist; must be a directory
  // donefile must exist; should be a file; not inside of srcspath
  // TODO: Fix hardcoded paths; client should specify
  std::string srcspath = "/home/holstgr/Development/Inshimtu/build/testing";
  std::string donefilepath = srcspath + ".done";

  inotify_descriptor = inotify_init();

  if ( inotify_descriptor < 0)
  {
    std::cerr << "Failed: Cannot init inotify filesystem watcher" << std::endl;
    return;
  }

  done_descriptor = inotify_add_watch( inotify_descriptor, donefilepath.c_str()
                                     , IN_ACCESS | IN_MODIFY | IN_ATTRIB | IN_DELETE_SELF );

  if ( done_descriptor < 0)
  {
    std::cerr << "Failed: Cannot add 'done' watch. Ensure file '"
              << donefilepath << "' exists." << std::endl;
    return;
  }

  watch_descriptor = inotify_add_watch( inotify_descriptor, srcspath.c_str()
                                      , IN_CLOSE_WRITE );

  if ( watch_descriptor < 0)
  {
    std::cerr << "Failed: Cannot add filesystem watcher."
              << "Ensure directory '" << srcspath << "' exists." << std::endl;
    remove_watches();
    return;
  }
}

INotify::~INotify()
{
  remove_watches();

  if (inotify_descriptor >=0)
  {
    close( inotify_descriptor );
    inotify_descriptor = -1;
  }

  std::cout << "FINALIZED INotify." << std::endl;
}

void INotify::processEvents(std::vector<std::string>& out_newFiles)
{
  assert(done_descriptor > 0 && "'done' event already recieved.");

  constexpr size_t EVENT_SIZE = sizeof (struct inotify_event);
  constexpr size_t BUF_LEN = 1024 * ( EVENT_SIZE + 16 );

  bool done = false;
  int i = 0;
  char buffer[BUF_LEN];
  const int length = read( inotify_descriptor, buffer, BUF_LEN );

  if ( length < 0 )
  {
    std::cerr << "WARNING: INotify read failed." << std::endl;
    return;
  }

  while ( i < length )
  {
    struct inotify_event* event = static_cast<struct inotify_event*>(static_cast<void*>(&buffer[i]));
    if (is_watched_event(*event) && is_closed_file(*event))
    {
      std::cout << "Watched file '" << event->name << "' was closed.\n" << std::endl;

      // add to client's newly found files
      out_newFiles.push_back(event->name);

      // add to our bookkeeping list
      auto fitr = std::find(found_files.begin(), found_files.end(), std::string(event->name));
      if (fitr == found_files.end())
      {
        found_files.push_back(event->name);
        std::cout << "Added newly found file '" << event->name << "' to found files.\n" << std::endl;
      }
    }
    else if (is_done_event(*event))
    {
      done = true;

      // TODO: Fix hardcoded paths; client should specify
      std::string srcspath = "/home/holstgr/Development/Inshimtu/build/testing";
      std::string donefilepath = srcspath + ".done";
      std::cout << "DONE Event! file '" << donefilepath << "' was modified.\n" << std::endl;
    }
    i += EVENT_SIZE + event->len;
  }

  if (done)
  {
    remove_watches();
  }
}

void INotify::remove_watches()
{
  if (done_descriptor >= 0)
  {
    inotify_rm_watch( inotify_descriptor, done_descriptor );
    done_descriptor = -1;
  }

  if (watch_descriptor >= 0)
  {
    inotify_rm_watch( inotify_descriptor, watch_descriptor );
    watch_descriptor = -1;
  }
}

