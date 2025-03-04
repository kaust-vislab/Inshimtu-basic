/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */

 #include "sentinels/notification.h"
 #include "utils/logger.h"
 
 #include <iostream>
 #include <algorithm>
 #include <fstream>
 #include <chrono>
 #include <thread>

 #include <boost/filesystem.hpp>
 #include <boost/regex.hpp>
 
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
 
 PollingNotify::PollingNotify( const std::vector<InputSpecPaths>& watch_paths
                             , const fs::path& done)
   : Notify(), done_file(done), polling_interval(2), done_flag(false),
     monitoring_start_time(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()))
 {
   BOOST_LOG_TRIVIAL(trace) << "STARTED PollingNotify";
 
   if (!fs::is_regular_file(done_file))
   {
     BOOST_LOG_TRIVIAL(error) << "Failed: Cannot set polling 'done'. Expected an existing file. "
                              << "Ensure file '" << done_file << "' exists.";
     done_flag = true;
     return;
   }
 
   for (const auto& inSp: watch_paths)
   {
     if (!fs::is_directory(inSp.directory))
     {
       BOOST_LOG_TRIVIAL(warning) << "Failed: Cannot set polling 'watch'. Expected an existing directory. "
                                << "Ensure directory '" << inSp.directory << "' exists.";
       continue;
     }
 
     if (fs::equivalent(done_file.parent_path(), inSp.directory))
     {
       BOOST_LOG_TRIVIAL(warning) << "Failed: Cannot set polling 'done'. Expected file outside of 'watch' directory. "
                                << "Ensure file '" << done_file << "' is not within '"
                                << inSp.directory << "' directory.";
       continue;
     }
 
     auto it = std::find_if(watches.begin(), watches.end(),
                            [&](const WatchSpec& w) { return inSp.directory == w.directory; });
 
     if (it == watches.end())
     {
       watches.emplace_back(inSp.directory, inSp.filenames);
       BOOST_LOG_TRIVIAL(info) << "Added filter '" << inSp.filenames << "' and watch directory '" << inSp.directory << "'.";
     }
     else
     {
       it->files_filters.push_back(inSp.filenames);
       BOOST_LOG_TRIVIAL(info) << "Added filter '" << inSp.filenames << "' to watch directory '" << it->directory << "'.";
     }
   }
 }
 
 PollingNotify::~PollingNotify()
 {
   BOOST_LOG_TRIVIAL(trace) << "FINALIZED PollingNotify.";
 }
 
 void PollingNotify::processEvents(std::vector<fs::path>& out_newFiles)
 {
   if (done_flag)
     return;
 
   for (auto& watch : watches)
   {
     for (fs::directory_iterator itr(watch.directory), end; itr != end; ++itr)
     {
       if (!fs::is_regular_file(itr->path()))
         continue;
 
       const std::string filename = itr->path().filename().string();
       auto last_write_time = fs::last_write_time(itr->path());
 
       if (last_write_time <= monitoring_start_time)
         continue;
 
       for (const auto& filter : watch.files_filters)
       {
         if (boost::regex_match(filename, filter))
         {
           auto known = watch.known_files.find(filename);
           if (known == watch.known_files.end() || known->second != last_write_time)
           {
             out_newFiles.push_back(itr->path());
             watch.known_files[filename] = last_write_time;
             BOOST_LOG_TRIVIAL(debug) << "Detected new/updated file: " << itr->path();
           }
         }
       }
     }
   }
   done_flag = checkDone();
   std::this_thread::sleep_for(std::chrono::seconds(polling_interval));
 }
 
 bool PollingNotify::isDone() const
 {
   return done_flag;
 }
 
 bool PollingNotify::checkDone() const
 {
   return !fs::exists(done_file);
 }