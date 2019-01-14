#ifndef COORDINATOR_HEADER
#define COORDINATOR_HEADER

#include "application.h"

#include <vector>
#include <map>
#include <set>

#include <boost/filesystem.hpp>

#include <unistd.h>
#include <sys/types.h>

#include <vtkNew.h>
#include <vtkMPICommunicator.h>


class Configuration;
class Notify;
class vtkMPICommunicatorOpaqueComm;


class Coordinator
{
public:
  Coordinator(MPICatalystApplication& app, const Notify& notify, const Configuration& configs);
  virtual ~Coordinator();

  enum FileNotificationType
  {
    InitialFiles
  , WatchedFiles
  };

  //! \brief  Global update function
  //! \arg newfiles   List of files locally notified as ready
  //! \arg ftype      File notification type; InitialFiles are immediately ready
  //!                 WatchedFiles become ready when all nodes agree
  //! \return         true if update does not need to be called again and notifiers
  //!                 agree they are done (still need to process readyFiles);
  //!                 false if future updates needed.
  //! Call update periodically with new files collected from notifier.
  //! After each invocation, inporter nodes should process ALL the ready files.
  //! If update returns true, then coordinated notification is complete.
  bool update( const std::vector<boost::filesystem::path>& newfiles
             , FileNotificationType ftype = WatchedFiles);

  bool anyReadyFiles() const;
  const std::vector<boost::filesystem::path>& getReadyFiles() const { return readyFiles; }

protected:
  MPICatalystApplication& app;
  const Notify& notify;
  const Configuration& configs;

  struct FileInfo
  {
    FileInfo()
      : writers()
      , is_initial(false)
      , is_ready(false)
      , is_signal(false)
      , was_signalled(false)
      , was_processed(false)
      , modified(INVALID_TIMEIVAL)
    {}

    void setModified(size_t currenttick)
    {
      // assumes that currenttick is non-decreasing
      if (modified == INVALID_TIMEIVAL)
      {
        modified = TimeInterval(currenttick, currenttick);
      }
      else
      {
        modified.second = currenttick;
      }
    }

    typedef std::pair<size_t,size_t> TimeInterval;
    static const TimeInterval INVALID_TIMEIVAL;

    std::set<MPISection::NodeRank> writers; // nodes that have written this file
    TimeInterval modified;
    bool is_initial;
    bool is_ready;
    bool is_signal;
    bool was_signalled;
    bool was_processed;
  };

  typedef std::map<boost::filesystem::path, FileInfo> FileInfoMap;

  size_t collectTick;
  std::set<MPISection::NodeRank> writers; // every node found to write a file

  FileInfoMap filesInfo;
  std::vector<boost::filesystem::path> readyFiles;

  size_t getTotalFiles(const std::vector<boost::filesystem::path>& files) const;
  bool isDone() const;

  //! \brief Provides corresponding data output file from possible signal file path
  //! \arg path   Filepath
  //! \return   Optional filepath which is nothing if input path is not a signal file
  boost::optional<boost::filesystem::path> getSignalledOutputFile(const boost::filesystem::path& path) const;

private:

  template<typename T>
  T update_updateProcessedFiles();
  template<typename S, typename T>
  T update_gatherNewFiles( S
                         , const std::vector<boost::filesystem::path>& newfiles
                         , FileNotificationType ftype
                         , const size_t total_global );
  template<typename S, typename T>
  T update_calculateReadyFiles( S, const bool is_done_global );
  template<typename S>
  void update_broadcastReadyFiles(S);
};

#endif
