/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#ifndef CORE_SPECIFICATIONS_HEADER
#define CORE_SPECIFICATIONS_HEADER

#include <iostream>
#include <vector>
#include <string>

#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <boost/variant.hpp>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>




// TODO: explore notes and type representations of workflows
/*
enum class Responsibility {
  Coordinator = 0x0 // rank-0 mangager; only 1 total
, INotify // only 1 per node; each simulation writer node must have 1
, IPoll // only 1 per node; poll private filesystem, e.g., /dev/shm
, SPoll // only 1 total; poll shared filesystem, e.g., /scratch
, CatalystLiveRemote // only 1 total; Inports ready files from shared filesystem, transmits to ParaView
, CatalystWriter // Inports ready files, writes into shared filesystem
};

enum class FileProcessPolicy {
  ProcessOnClose // requires INotify event
, ProcessAfterAllWritersOpenAndClose // requires INotify event
, ProcessOnNextTimeStep // ses either INotify or ?Poll, but requires name policy
};

enum class FilePostProcessPolicy {
  DeleteWhenDone // all processing with it must be completed
, MoveWhenDone // all processing with it must be completed
, ExternalActionWhenDone // calls a post processing tool
};

enum class FileOrganization {
  SingleFilePerTimestep
, FragmentedFilesPerTimestep
, SingleCompoundFileTotalAllTimesteps // must be data format like NetCDF or HDF5 which allows parallel reading, must contain time step, and allow access indexed by time step
, FragmentedCompoundFileTotalAllTimesteps // must be data format like NetCDF or HDF5 which allows parallel reading, must contain time step, and allow access indexed by time step
};

enum class FileState {
  OpenedByNodeDict
, ClosedByNodeDict
, IsReady
, NeedsProcessing
, GetData
, WasProcessed
, IsDone
};

// type Workflows =
//   OnFileReady -> InFile -> PreProcess -> Process -> Inport -> CatalystWriter -> OutFile -> PostProcess
//   OnFileReady -> InFile -> PreProcess -> CatalystLiveRemote -> PostProcess
//   OnFileReady -> InFile -> PreProcess -> Process -> Inport -> CatalystLiveRemote -> OutFile -> PostProcess
*/


// -- types --

typedef std::pair<boost::regex, std::string> ReplaceRegexFormat;


// -- input --

struct InputSpecPaths
{
  InputSpecPaths(const boost::filesystem::path& dir, const boost::regex& filemask);

  bool match(const boost::filesystem::path& filename) const;

  boost::filesystem::path directory;
  boost::regex filenames;
};


#endif

