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


class Notify
{
public:
  Notify();
  virtual ~Notify();

  virtual void processEvents(std::vector<boost::filesystem::path>& out_newFiles);
  virtual bool isDone() const;
};

class INotify : public Notify
{
public:
  INotify( const boost::filesystem::path& watch_directory
         , const boost::regex& watch_files_filter
         , const boost::filesystem::path& done_file);
  virtual ~INotify();

  virtual void processEvents(std::vector<boost::filesystem::path>& out_newFiles) override;

  virtual bool isDone() const override;

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
