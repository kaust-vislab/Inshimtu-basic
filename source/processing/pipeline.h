/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#ifndef PROCESSING_PIPELINE_HEADER
#define PROCESSING_PIPELINE_HEADER

#include "core/specifications.h"

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

struct InputSpecAny
{
  bool accept( const std::vector<boost::filesystem::path>& available
             , std::vector<boost::filesystem::path>& outAccepted) const;
};

typedef boost::variant< InputSpecPaths
                      , InputSpecAny > InputSpec;


// -- process --

struct ProcessingSpecReadyFile
{
  ProcessingSpecReadyFile(const ReplaceRegexFormat& convert);

  boost::optional<boost::filesystem::path> get(const boost::filesystem::path& filename) const;

  ReplaceRegexFormat conversion;
};

struct ProcessingSpecCatalyst
{
  ProcessingSpecCatalyst( const std::vector<boost::filesystem::path>& scripts_
                        , const std::vector<std::string>& variables_);

  void process( const boost::filesystem::path &filename
              , Descriptor& descriptor) const;

  std::vector<boost::filesystem::path> scripts;
  std::vector<std::string> variables;
};

struct ProcessingSpecCommands
{
  static const std::string FILENAME_ARG; //"$FILENAME";
  static const std::string FILENAMES_ARRAY_ARG; //"$FILENAMES_ARRAY";

  /// ProcessCommands_All x ProcessFiles_All: Each command is run with all files
  /// ProcessCommands_Separate x ProcessFiles_All: Each command is run with all files
  /// ProcessCommands_All x ProcessFiles_Single: Each file has all commands run on it
  /// ProcessCommands_Separate x ProcessFiles_Single: For each command and for each file the command is run on the file
  enum ProcessCommandsType
  {
    ProcessCommands_All
  , ProcessCommands_Separate
  };

  enum ProcessFilesType
  {
    ProcessFiles_All
  , ProcessFiles_Single
  };


  typedef boost::filesystem::path Exe;
  typedef std::vector<std::string> Args;
  typedef std::pair<Exe, Args> Command;

  ProcessingSpecCommands(const std::vector<Command>& cmds_);

  void setProcessingType(ProcessCommandsType pCmds, ProcessFilesType pFiles);

  //bool process(const boost::filesystem::path& filename) const;
  bool process(const std::vector<boost::filesystem::path>& files) const;

  std::vector<Command> commands;
  ProcessCommandsType processCommandsBy;
  ProcessFilesType processFilesBy;

protected:
  bool processCommand( const Command& cmd
                     , const std::vector<boost::filesystem::path>& files) const;
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
  PipelineSpec( const std::string& name_
              , InputSpec input_, ProcessingSpec process_, OutputSpec out_);

  std::string name;

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

  std::vector<boost::filesystem::path> inputFiles;

  std::vector<boost::filesystem::path> outputFiles;

  // ProcessingSpecCatalyst state
  typedef std::function<std::unique_ptr<Descriptor>()> MkDescriptorFn;
  boost::optional<MkDescriptorFn> mkDescriptor;
};


bool pipeline_AcceptInput( const PipelineSpec& pipeS
                         , const std::vector<boost::filesystem::path>& available
                         , std::vector<boost::filesystem::path>& outAccepted);

std::unique_ptr<TaskState> pipeline_MkPipelineTask( const PipelineSpec& pipeS
                                                  , const std::vector<boost::filesystem::path>& working
                                                  , TaskState::MkDescriptorFn mkDescriptor);

void pipeline_ProcessTask(std::unique_ptr<TaskState>& taskS);


#endif

