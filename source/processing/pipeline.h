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


class Descriptor;

struct PipelineSpec;
struct Attributes;
struct TaskState;


// -- input --

struct InputSpecAny
{
  InputSpecAny();

  bool accept( const std::vector<boost::filesystem::path>& available
             , std::vector<boost::filesystem::path>& outAccepted) const;

  void setAcceptFirst();
  void setAcceptAll();

  InputSpecPaths::AcceptType acceptType;
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
  typedef boost::filesystem::path Script;
  typedef std::string Variables;
  typedef std::pair<Script, Variables> ScriptSpec;

  ProcessingSpecCatalyst(const std::vector<ScriptSpec>& scripts_);

  void process( const boost::filesystem::path &filename
              , Descriptor& descriptor) const;

  std::vector<ScriptSpec> scripts;
};

struct ProcessingSpecCommands
{
  static const std::string FILENAME_ARG; //"$FILENAME";
  static const std::string FILENAMES_ARRAY_ARG; //"$FILENAMES_ARRAY";
  static const std::string TIMESTEP_CODE_ARG; //"${TIMESTEP_CODE}";

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

  bool process( const Attributes& attributes
              , const std::vector<boost::filesystem::path>& files) const;

  std::vector<Command> commands;
  ProcessCommandsType processCommandsBy;
  ProcessFilesType processFilesBy;

protected:
  bool processCommand( const Attributes& attributes
                     , const Command& cmd
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
};

typedef boost::variant< OutputSpecDone
                      , OutputSpecPipeline > OutputSpec;


// -- pipeline --

struct PipelineStage
{
  PipelineStage( const std::string& name_
               , InputSpec input_, ProcessingSpec process_, OutputSpec out_);

  std::string name;

  InputSpec input;
  ProcessingSpec process;
  OutputSpec out;
};

struct PipelineSpec
{
  PipelineSpec( const std::string& name_
              , InputSpec input_, ProcessingSpec process_, OutputSpec out_);
  PipelineSpec( const PipelineStage& stage_);
  PipelineSpec( const std::string& name_
              , const std::vector<PipelineStage>& stages_);

  const InputSpec getInput() const;

  std::string name;

  std::vector<PipelineStage> stages;
};


// -- runtime attributes --

struct Attributes
{
  typedef std::string AttributeKey;
  typedef boost::variant< std::string
                        , boost::filesystem::path> AttributeValue;
  typedef std::map<AttributeKey, AttributeValue> AttributeMap;


  bool hasAttribute(const AttributeKey& key) const;
  boost::optional<AttributeValue> getAttribute(const AttributeKey& key) const;
  void setAttribute(const AttributeKey& key, const AttributeValue& value);


  AttributeMap attributes;
};

// -- task --

struct TaskState
{
  TaskState(const PipelineSpec& pipeline_);

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

  bool hasAttribute(const Attributes::AttributeKey& key) const;
  boost::optional<Attributes::AttributeValue> getAttribute(const Attributes::AttributeKey& key) const;
  void setAttribute(const Attributes::AttributeKey& key, const Attributes::AttributeValue& value);

  boost::optional<PipelineStage> getStage() const;

  bool nextStage();


  TaskStatus taskStatus;

  PipelineSpec pipeline;
  std::vector<PipelineStage>::const_iterator stageItr;

  std::vector<boost::filesystem::path> inputFiles;
  std::vector<boost::filesystem::path> outputFiles;

  Attributes attributes;


  // ProcessingSpecCatalyst state
  typedef std::function<std::unique_ptr<Descriptor>()> MkDescriptorFn;
  boost::optional<MkDescriptorFn> mkDescriptor;
};


bool pipeline_AcceptInput( const InputSpec& inputS
                         , const std::vector<boost::filesystem::path>& available
                         , std::vector<boost::filesystem::path>& outAccepted
                         , Attributes& outAttributes);

std::unique_ptr<TaskState> pipeline_MkPipelineTask( const PipelineSpec& pipeS
                                                  , const std::vector<boost::filesystem::path>& working
                                                  , const Attributes& attributes
                                                  , TaskState::MkDescriptorFn mkDescriptor);

void pipeline_ProcessTask(std::unique_ptr<TaskState>& taskS);


#endif

