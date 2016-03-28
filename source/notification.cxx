#include "notification.h"
#include "logger.h"

#include <iostream>
#include <algorithm>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>


namespace fs = boost::filesystem;


INotify::INotify( const fs::path& watch
                , const boost::regex& mask
                , const fs::path& done)
  : inotify_descriptor(-1)
  , watch_descriptor(-1)
  , done_descriptor(-1)
  , watch_directory(watch)
  , watch_files_filter(mask)
  , done_file(done)
{
  std::cout << "STARTED INotify" << std::endl;

  if (!fs::is_directory(watch_directory))
  {
    std::cerr << "Failed: Cannot set inotify 'watch'. Expected an exisiting directory. "
              << " Ensure directory '" << watch_directory << "' exists." << std::endl;
    return;
  }

  if (!fs::is_regular_file(done_file))
  {
    std::cerr << "Failed: Cannot set inotify 'done'. Expected an exisiting file. "
              << " Ensure file '" << done_file << "' exists." << std::endl;
    return;
  }

  if (fs::equivalent(done_file.parent_path(), watch_directory))
  {
    std::cerr << "Failed: Cannot set inotify 'done'. Expected file outside of 'watch' directory. "
              << " Ensure file '" << done_file << "' is not within '"
              << watch_directory << "' directory." << std::endl;
    return;
  }

  inotify_descriptor = inotify_init();

  if ( inotify_descriptor < 0)
  {
    std::cerr << "Failed: Cannot init inotify filesystem watcher" << std::endl;
    return;
  }

  done_descriptor = inotify_add_watch( inotify_descriptor, done_file.c_str()
                                     , IN_ACCESS | IN_MODIFY | IN_ATTRIB | IN_DELETE_SELF );

  if ( done_descriptor < 0)
  {
    std::cerr << "Failed: Cannot add 'done' watch. Ensure file '"
              << done_file << "' exists." << std::endl;
    return;
  }

  watch_descriptor = inotify_add_watch( inotify_descriptor, watch_directory.c_str()
                                      , IN_CLOSE_WRITE );

  if ( watch_descriptor < 0)
  {
    std::cerr << "Failed: Cannot add filesystem watcher."
              << "Ensure directory '" << watch_directory << "' exists." << std::endl;
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

void INotify::processEvents(std::vector<fs::path>& out_newFiles)
{
  assert(done_descriptor > 0 && "'done' event already recieved.");

  constexpr size_t EVENT_SIZE = sizeof (struct inotify_event);
  constexpr size_t BUF_LEN = 1024 * ( EVENT_SIZE + 16 );

  bool done = false;
  int i = 0;
  char buffer[BUF_LEN];

  if (!isReady())
  {
    // nothing yet
    return;
  }

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
      if (boost::regex_match(event->name, watch_files_filter))
      {
        const fs::path name(watch_directory / event->name);

        std::cout << "Watched file '" << name << "' was closed.\n" << std::endl;

        // add to client's newly found files
        out_newFiles.push_back(name);

        // add to our bookkeeping list
        auto fitr = std::find(found_files.begin(), found_files.end(), name);
        if (fitr == found_files.end())
        {
          found_files.push_back(name);
          std::cout << "Added newly found file '" << name << "' to found files.\n" << std::endl;
        }
      }
    }
    else if (is_done_event(*event))
    {
      done = true;
      std::cout << "DONE Event! file '" << done_file << "' was modified.\n" << std::endl;
    }
    i += EVENT_SIZE + event->len;
  }

  if (done)
  {
    remove_watches();
  }
}

// TODO: Generalize how done is signalled (not just by writing a file,
//       but other options too, e.g., co-process has ended)
bool INotify::isDone() const
{
  return done_descriptor < 0;
}

bool INotify::isReady() const
{
  const __time_t secs = 1;
  timeval timeout{secs,0};
  fd_set r_fds, w_fds, x_fds;
  FD_ZERO(&r_fds);
  FD_ZERO(&w_fds);
  FD_ZERO(&x_fds);
  FD_SET(inotify_descriptor, &r_fds);

  const int result = select(inotify_descriptor+1, &r_fds, &w_fds, &x_fds, &timeout);

  return result > 0;
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

