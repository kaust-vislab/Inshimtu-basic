/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#include "sentinels/notification.h"
#include "utils/logger.h"

#include <iostream>
#include <algorithm>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>


namespace fs = boost::filesystem;


Notify::Notify()
{
  BOOST_LOG_TRIVIAL(trace) << "STARTED Notification";
}

Notify::~Notify()
{
  BOOST_LOG_TRIVIAL(trace) << "FINALIZED Notification.";
}

void Notify::processEvents(std::vector<boost::filesystem::path>&)
{
}

bool Notify::isDone() const
{
  return true;
}


INotify::INotify( const std::vector<InputSpecPaths>& watch_paths
                , const fs::path& done)
  : Notify()
  , inotify_descriptor(-1)
  , done_descriptor(-1)
  , done_file(done)
  , watches()
{
  BOOST_LOG_TRIVIAL(trace) << "STARTED INotify";

  if (!fs::is_regular_file(done_file))
  {
    BOOST_LOG_TRIVIAL(error) << "Failed: Cannot set inotify 'done'. Expected an exisiting file. "
                             << " Ensure file '" << done_file << "' exists.";
    return;
  }

  inotify_descriptor = inotify_init();

  if (inotify_descriptor < 0)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed: Cannot init inotify filesystem watcher";
    return;
  }

  done_descriptor = inotify_add_watch( inotify_descriptor, done_file.c_str()
                                     , IN_ACCESS | IN_MODIFY | IN_ATTRIB | IN_DELETE_SELF );

  if (done_descriptor < 0)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed: Cannot add 'done' watch. Ensure file '"
                             << done_file << "' exists.";
    return;
  }

  for (const auto& inSp: watch_paths)
  {

    if (!fs::is_directory(inSp.directory))
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed: Cannot set inotify 'watch'. Expected an exisiting directory. "
                               << " Ensure directory '" << inSp.directory << "' exists.";
      continue;
    }

    if (fs::equivalent(done_file.parent_path(), inSp.directory))
    {
      BOOST_LOG_TRIVIAL(warning) << "Failed: Cannot set inotify 'done'. Expected file outside of 'watch' directory. "
                               << " Ensure file '" << done_file << "' is not within '"
                               << inSp.directory << "' directory.";
      continue;
    }

    auto it = std::find_if( std::begin(watches), std::end(watches)
                          , [&] (const WatchSpec& w) { return inSp.directory == w.directory; } );

    if (it == std::end(watches))
    {
      WatchSpec ws(inSp.directory, inSp.filenames);

      ws.descriptor = inotify_add_watch( inotify_descriptor, ws.directory.c_str()
                                       , IN_CLOSE_WRITE );

      if (ws.descriptor < 0)
      {
        BOOST_LOG_TRIVIAL(warning) << "Failed: Cannot add filesystem watcher."
                                 << "Ensure directory '" << ws.directory << "' exists.";
        ws.descriptor = -1;
        continue;
      }

      watches.push_back(ws);

      BOOST_LOG_TRIVIAL(info) << "Added filter '" << inSp.filenames << "' and watch directory '" << ws.directory << "'.";
    }
    else
    {
      WatchSpec& ws(*it);

      ws.files_filters.push_back(inSp.filenames);

      BOOST_LOG_TRIVIAL(info) << "Added filter '" << inSp.filenames << "' to watch directory '" << ws.directory << "'.";
    }
  }

  if (watches.empty())
  {
    remove_watches();

    BOOST_LOG_TRIVIAL(error) << "Failed: No valid filesystem watchers found for notification.";
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
}

void INotify::processEvents(std::vector<fs::path>& out_newFiles)
{
  assert(done_descriptor > 0 && "'done' event already recieved.");

  constexpr size_t EVENT_SIZE = sizeof (struct inotify_event);
  constexpr size_t BUF_LEN = 1024 * ( EVENT_SIZE + 16 );

  bool done = false;
  char buffer[BUF_LEN];

  if (!isReady())
  {
    // nothing yet
    return;
  }

  const long length = read( inotify_descriptor, buffer, BUF_LEN );

  if ( length < 0 )
  {
    BOOST_LOG_TRIVIAL(warning) << "WARNING: INotify read failed.";
    return;
  }

  int i = 0;
  while ( i < length )
  {
    struct inotify_event* event = static_cast<struct inotify_event*>(static_cast<void*>(&buffer[i]));

    boost::optional<const WatchSpec> ows(get_watch_spec(*event));

    if (ows.is_initialized())
    {
      const WatchSpec& watch(ows.get());

      if (is_closed_file(*event))
      {
        BOOST_LOG_TRIVIAL(debug) << "Event IS a closed file: '" << event->name << " :: " << event->mask << "\n";
        const fs::path name(watch.directory / event->name);
        for(const auto& files_filter: watch.files_filters)
        {
            if (boost::regex_match(event->name, files_filter))
            {
              BOOST_LOG_TRIVIAL(debug) << "Watched file '" << name << "' matched regex.\n";

              // add to client's newly found files
              out_newFiles.push_back(name);

              // add to our bookkeeping list
              auto fitr = std::find(std::begin(found_files), std::end(found_files), name);
              if (fitr == std::end(found_files))
              {
                found_files.push_back(name);
                BOOST_LOG_TRIVIAL(debug) << "Added newly found file '" << name << "' to found files.\n";
              }
            }
            else
            {
              BOOST_LOG_TRIVIAL(debug) << "Event name does not match the files_filter. Event name/mask/filter (" << event->name << " :: " << event->mask << " :: " << files_filter << ")\n";
            }
        }
      }
      else
      {
        BOOST_LOG_TRIVIAL(debug) << "Event is NOT a closed file: '" << event->name << " :: " << event->mask << "\n";
      }
    }

    if (is_done_event(*event))
    {
      done = true;
      BOOST_LOG_TRIVIAL(debug) << "DONE Event! file '" << done_file << "' was modified.\n";
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

  for(auto& w: watches)
  {
    if (w.descriptor >= 0)
    {
      inotify_rm_watch( inotify_descriptor, w.descriptor );
      w.descriptor = -1;
    }
  }
}

