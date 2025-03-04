/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#ifndef SENTINELS_NOTIFICATION_HEADER
#define SENTINELS_NOTIFICATION_HEADER

#include "core/options.h"
#include "core/specifications.h"

#include "sentinels/notification.h"
#include "utils/logger.h"

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <unordered_set>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

namespace fs = boost::filesystem;
class Notify
{
public:
  Notify();
  virtual ~Notify();

  virtual void processEvents(std::vector<boost::filesystem::path>& out_newFiles);
  virtual bool isDone() const;
};


class PollingNotify : public Notify {
  public:
      PollingNotify(const std::vector<InputSpecPaths>& watch_paths, const fs::path& done);
      ~PollingNotify();
  
      void processEvents(std::vector<fs::path>& out_newFiles) override;
      bool isDone() const override;
  
  private:
      fs::path done_file;
      std::vector<fs::path> watch_dirs;
      std::vector<std::string> known_files;
      time_t last_mtime; // Stores last modification time of done_file
      bool done_flag;
  
      bool fileExists(const fs::path& file);
      time_t getFileMTime(const fs::path& file);
  };

class INotify : public Notify
{
public:
  INotify( const std::vector<InputSpecPaths>& watch_paths
         , const boost::filesystem::path& done_file);
  virtual ~INotify() override;

  virtual void processEvents(std::vector<boost::filesystem::path>& out_newFiles) override;

  virtual bool isDone() const override;

protected:

  int inotify_descriptor;
  int done_descriptor;
  const boost::filesystem::path done_file;

  std::vector<boost::filesystem::path> found_files;

  struct WatchSpec
  {
    WatchSpec() : descriptor(-1) {}
    WatchSpec( const boost::filesystem::path watch_directory
             , const boost::regex& watch_files)
      : descriptor(-1)
      , directory(watch_directory)
      , files_filters({watch_files})
    {
    }

    int descriptor;
    const boost::filesystem::path directory;
    std::vector<boost::regex> files_filters;
  };

  std::vector<WatchSpec> watches;

private:

  bool isReady() const;

  static bool is_closed_file(const struct inotify_event& event)
  {
    return (  event.len > 0
           && (event.mask & IN_CLOSE_WRITE) != 0
           && (event.mask & IN_ISDIR) == 0);
  }

  bool is_watched_event(const struct inotify_event& event) const
  {
    return (get_watch_spec(event).is_initialized());
  }

  boost::optional<const WatchSpec> get_watch_spec(const struct inotify_event& event) const
  {
    auto it = find_if( std::begin(watches), std::end(watches)
                     , [&] (const WatchSpec& w) { return event.wd == w.descriptor; } );
    if (it != std::end(watches))
    {
      return boost::optional<const WatchSpec>(*it);
    }

    return boost::optional<const WatchSpec>();
  }

  bool is_done_event(const struct inotify_event& event) const
  {
    return (event.wd == done_descriptor);
  }

  void remove_watches();
};

#endif
