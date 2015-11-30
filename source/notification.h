#ifndef NOTIFICATION_HEADER
#define NOTIFICATION_HEADER

#include <vector>
#include <string>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>

class INotify
{
public:
  INotify();
  ~INotify();

  void processEvents(std::vector<std::string>& out_newFiles);

  bool isDone() const { return done_descriptor < 0; }

protected:
  int inotify_descriptor;
  int done_descriptor;
  int watch_descriptor;

  std::vector<std::string> found_files;

private:

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
