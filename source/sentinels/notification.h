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
#include <unordered_map>
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
  virtual void processEvents(std::vector<std::string>& out_newFiles);
  virtual bool isDone() const;
  virtual bool isDonePolling();
};


class PollingNotify : public Notify
{
public:
    PollingNotify(const std::vector<std::string>& watch_paths, const std::string& done_file);
    virtual void processEvents(std::vector<std::string>& newfiles) override;
    virtual bool isDonePolling() override;

private:
    struct FileInfo {
        time_t mtime;
    };

    std::vector<std::string> watch_dirs;  // Directories being monitored
    std::unordered_map<std::string, FileInfo> known_files;  // Track file changes
    std::string done_file;
    time_t done_file_mtime;

    void scanDirectory(const std::string& dir, std::vector<std::string>& newfiles);
    time_t getFileMtime(const std::string& filepath);
};

class INotify : public Notify
{
public:
  INotify( const std::vector<InputSpecPaths>& watch_paths
         , const boost::filesystem::path& done_file);
  virtual ~INotify() override;

  virtual void processEvents(std::vector<boost::filesystem::path>& out_newFiles) override;

  virtual bool isDone() const override;

  void processEvents(std::vector<std::string>& newfiles) override {}

  bool isDonePolling() override { return false;}

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
