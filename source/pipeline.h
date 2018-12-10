/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#ifndef PIPELINE_HEADER
#define PIPELINE_HEADER

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

class Descriptor;

struct PipelineSpec;


// -- input --

struct InputSpecPaths
{
  InputSpecPaths(const boost::filesystem::path& dir, const boost::regex& filemask);

  bool match(const boost::filesystem::path& filename) const;

  boost::filesystem::path directory;
  boost::regex filenames;
};

struct InputSpecPipeline
{
};

typedef boost::variant< InputSpecPaths
                      , InputSpecPipeline > InputSpec;


// -- process --

typedef std::pair<boost::regex, std::string> ReplaceRegexFormat;

struct ProcessingSpecReadyFile
{
  ProcessingSpecReadyFile(const ReplaceRegexFormat& convert);

  boost::optional<boost::filesystem::path> get(const boost::filesystem::path& filename) const;

  ReplaceRegexFormat conversion;
};

struct ProcessingSpecCatalyst
{
  std::vector<boost::filesystem::path> scripts;
  std::vector<std::string> variables;
};

struct ProcessingSpecCommands
{
  std::vector<boost::filesystem::path> commands;
};

typedef boost::variant< ProcessingSpecReadyFile
                      , ProcessingSpecCatalyst
                      , ProcessingSpecCommands > ProcessingSpec;


// -- output --

struct OutputSpecDone
{
  OutputSpecDone();

  bool deleteInput;
};
struct OutputSpecPipeline
{
  OutputSpecPipeline();

  bool deleteInput;
  const PipelineSpec* pipeline;
};

typedef boost::variant< OutputSpecDone
                      , OutputSpecPipeline > OutputSpec;


// -- pipeline --

struct PipelineSpec
{
  InputSpec input;
  ProcessingSpec process;
  OutputSpec out;
};


// -- task --

struct TaskState
{
  TaskState();

  enum TaskStatus
  {
    TS_OK
  , TS_Done
  , TS_Error
  , TS_FailedInput
  , TS_FailedProcessing
  };

  bool canContinue() const;
  bool wasSuccessful() const;
  bool hasError() const;

  TaskStatus taskStatus;

  boost::optional<PipelineSpec> stage;

  std::vector<boost::filesystem::path> readyFiles;

  std::vector<boost::filesystem::path> products;

  // ProcessingSpecCatalyst state
  typedef std::function<std::unique_ptr<Descriptor>()> MkDescriptorFn;
  boost::optional<MkDescriptorFn> mkDescriptor;
};


bool pipeline_AcceptInput( const PipelineSpec& pipeS
                         , const boost::filesystem::path& filename);
std::unique_ptr<TaskState> pipeline_MkPipelineTask( const PipelineSpec& pipeS
                                                  , const boost::filesystem::path& filename
                                                  , std::function<std::unique_ptr<Descriptor>()> mkDescriptor);
void pipeline_ProcessTask(std::unique_ptr<TaskState>& taskS);


#endif

