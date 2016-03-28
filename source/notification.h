#ifndef NOTIFICATION_HEADER
#define NOTIFICATION_HEADER

#include <vector>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

class INotify
{
public:
  INotify( const boost::filesystem::path& watch_directory
         , const boost::regex& watch_files_filter
         , const boost::filesystem::path& done_file);
  ~INotify();

  void processEvents(std::vector<boost::filesystem::path>& out_newFiles);

  bool isDone() const;

protected:
  int inotify_descriptor;
  int watch_descriptor;
  int done_descriptor;

  const boost::filesystem::path watch_directory;
  const boost::regex watch_files_filter;
  const boost::filesystem::path done_file;

  std::vector<boost::filesystem::path> found_files;

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
    return (event.wd == watch_descriptor);
  }

  bool is_done_event(const struct inotify_event& event) const
  {
    return (event.wd == done_descriptor);
  }

  void remove_watches();
};

#endif
