/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#ifndef SENTINELS_NOTIFICATION_HEADER
#define SENTINELS_NOTIFICATION_HEADER

#include "core/options.h"
#include "core/specifications.h"

#include <vector>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>


class Notify
{
public:
  Notify();
  virtual ~Notify();

  virtual void processEvents(std::vector<boost::filesystem::path>& out_newFiles);
  virtual bool isDone() const;
};

class PollingNotify : public Notify
{
public:
  PollingNotify( const std::vector<InputSpecPaths>& watch_paths
               , const boost::filesystem::path& done_file);
  virtual ~PollingNotify() override;

  virtual void processEvents(std::vector<boost::filesystem::path>& out_newFiles) override;
  virtual bool isDone() const override;

protected:
  const boost::filesystem::path done_file;
  std::vector<boost::filesystem::path> found_files;
  std::chrono::seconds polling_interval;

  struct WatchSpec
  {
    WatchSpec(const boost::filesystem::path& watch_directory, const boost::regex& watch_files)
      : directory(watch_directory), files_filters({watch_files}) {}

    boost::filesystem::path directory;
    std::vector<boost::regex> files_filters;
    std::map<std::string, std::time_t> known_files;
  };

  std::vector<WatchSpec> watches;
  bool done_flag;

private:
  bool checkDone() const;
  std::time_t monitoring_start_time;
};

#endif
