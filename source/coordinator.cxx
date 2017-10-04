#include "coordinator.h"
#include "logger.h"
#include "notification.h"
#include "options.h"

#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <mpi.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#include <vtkProcessGroup.h>
#include <vtkMPI.h>


namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace
{
const int MSGTAG_COLLECT_NEWFILES = 10;
}

const Coordinator::FileInfo::TimeInterval Coordinator::FileInfo::INVALID_TIMEIVAL = Coordinator::FileInfo::TimeInterval(1,0);


Coordinator::Coordinator(MPICatalystApplication& app_, const Notify& notify_, const Configuration& configs_)
  : app(app_)
  , notify(notify_)
  , configs(configs_)
  , collectTick(0)
  , writers()
  , filesInfo()
  , readyFiles()
{
}

Coordinator::~Coordinator()
{
}


size_t Coordinator::getTotalFiles(const std::vector<fs::path>& files) const
{
  size_t total_global = 0;
  const size_t total = files.size();

  // Note: The following logic assumes: a global filename space
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Allreduce(&total, &total_global, 1, MPI_UNSIGNED, MPI_SUM, MPI_COMM_WORLD);

  return total_global;
}

bool Coordinator::anyReadyFiles() const
{
  const size_t total_global(getTotalFiles(readyFiles));
  return  total_global > 0;
}

bool Coordinator::update( const std::vector<fs::path>& newfiles
                        , FileNotificationType ftype)
{
  char message[PATH_MAX];
  const size_t total_global = getTotalFiles(newfiles);
  const bool is_done_global = isDone();


  // Note: The following logic assumes:
  //          a global filename space,
  //          root node will coordinate,
  //          all ready files are processed prior to calling update again

  // update processed files
  {
    if (app.isRoot())
    {
      // NOTE: assume all previously ready ready files are now processed
      for (const auto& f: readyFiles)
      {
        filesInfo[f].was_processed = true;
      }
    }

    readyFiles.clear();
  }

  // early out if nothing to be done
  if (total_global == 0 && !is_done_global)
  {
    // nothing to collect
    return is_done_global;
  }

  ++collectTick;

  // root node gathers all new files (including root local)
  if (app.isRoot())
  {
    MPI_Status status;

    // newfiles from root node
    for (const auto& f : newfiles)
    {
      auto& info(filesInfo[f]);

      info.is_initial = ftype == InitialFiles;
      info.setModified(collectTick);
      info.writers.insert(app.getRank());
      writers.insert(app.getRank());
    }

    // root node doesn't contribute to collect newfile messages
    const int total_global_remaining = total_global - newfiles.size();

    // newfiles from other nodes
    for (int i = 0; i < total_global_remaining; ++i)
    {
      MPI_Recv( message, sizeof(message), MPI_CHAR
              , MPI_ANY_SOURCE, MSGTAG_COLLECT_NEWFILES, MPI_COMM_WORLD
              , &status);

      auto& info(filesInfo[fs::path(message)]);

      info.setModified(collectTick);
      info.writers.insert(status.MPI_SOURCE);
      writers.insert(status.MPI_SOURCE);
    }
  }
  else
  {
    for (const auto& f : newfiles)
    {
      MPI_Send( f.c_str(), f.string().size()+1, MPI_CHAR
              , MPIApplication::ROOT_RANK, MSGTAG_COLLECT_NEWFILES, MPI_COMM_WORLD);
    }
  }


  // process output ready signals
  if (app.isRoot())
  {
    std::vector<fs::path> signalledReady;

    for (auto&f : filesInfo)
    {
      const fs::path& fpath(f.first);
      FileInfo& finfo(f.second);

      if (!finfo.was_processed)
      {
        boost::optional<fs::path> outputFile(getSignalledOutputFile(fpath));

        if (outputFile.is_initialized())
        {
          finfo.is_ready = true;
          finfo.is_signal = true;

          signalledReady.push_back(outputFile.get());

          finfo.was_processed = true;
        }
      }
    }

    for (const auto& f : signalledReady)
    {
      auto& info(filesInfo[f]);

      info.setModified(collectTick);
      info.was_signalled = true;
    }
  }


  // re-calculate ready files
  {
    readyFiles.clear();

    if (app.isRoot())
    {
      const size_t filesinfoSize(filesInfo.size());
      size_t finfoIndex(0);

      for (auto&f : filesInfo)
      {
        const fs::path& fpath(f.first);
        FileInfo& finfo(f.second);

        // NOTE: isOlderFile test depends of filename ordering matching output
        ++finfoIndex;
        const bool isOlderFile(finfoIndex < filesinfoSize);

        if (!finfo.is_ready && !finfo.was_processed)
        {
          // initial files are always ready
          if (finfo.is_initial)
          {
            finfo.is_ready = true;
          }
          else
          // the corresponding data file should exist if output ready file seen
          if (finfo.was_signalled)
          {
            finfo.is_ready = true;
          }
          else
          // assumes we have seen complete set of writers after first file was written,
          // and assumes first file was written completely if:
          // we have seen the second file, or we've seen done notification (from anyone)
          if ( (filesinfoSize > 1 || is_done_global)
             && std::includes( finfo.writers.cbegin(), finfo.writers.cend()
                             , writers.cbegin(), writers.cend()))
          {
            finfo.is_ready = true;
          }
          else
          // assumes file was written completely if we have seen a newer file (from anyone)
          if (isOlderFile)
          {
            finfo.is_ready = true;
          }
          else
          // assumes done notification occurs only after simulation exits, and
          // data files are completely written out by then.
          if (is_done_global)
          {
            finfo.is_ready = true;
          }
        }

        if (finfo.is_ready && !finfo.was_processed)
        {
          readyFiles.push_back(fpath.string());
        }
      }
    }
  }


  // broadcast readyfiles to inporter nodes
  {
    MPI_Comm inporterComm = *app.getCommunicator().GetHandle();

    int readyCount;

    if (app.isRoot())
    {
      readyCount = readyFiles.size();
    }

    MPI_Bcast(&readyCount, 1, MPI_INT, MPIApplication::ROOT_RANK, inporterComm);

    for (int i = 0; i < readyCount; ++i)
    {
      if (app.isRoot())
      {
        const auto& f(readyFiles[i]);
        strncpy(message, f.c_str(), std::min(f.string().size()+1, static_cast<size_t>(PATH_MAX)));
      }

      MPI_Bcast(message, sizeof(message), MPI_CHAR, MPIApplication::ROOT_RANK, inporterComm);

      if (!app.isRoot())
      {
        readyFiles.push_back(fs::path(message));
      }
    }
  }

  // final synchronize for all nodes
  MPI_Barrier(MPI_COMM_WORLD);

  return is_done_global;
}

bool Coordinator::isDone() const
{
  const int DONE = 1;
  int done = notify.isDone() ? DONE : 0;
  int done_global = 0;

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Allreduce(&done, &done_global, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

  return done_global == DONE;
}

boost::optional<fs::path> Coordinator::getSignalledOutputFile(const fs::path& path) const
{
  boost::optional<fs::path> signalledPath;
  boost::optional<Configuration::ReplaceRegexFormat> conversion(configs.getOutputReadyConversion());

  if (conversion.is_initialized())
  {
    std::string signalledFile(boost::regex_replace( path.string()
                                                  , conversion->first, conversion->second
                                                  , boost::match_default | boost::format_default | boost::format_no_copy));

    if (!signalledFile.empty())
    {
      signalledPath = fs::path(signalledFile);
    }
  }

  return signalledPath;
}

