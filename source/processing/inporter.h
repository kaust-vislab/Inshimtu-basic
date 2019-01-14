/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#ifndef PROCESSING_INPORTER_HEADER
#define PROCESSING_INPORTER_HEADER

#include "core/options.h"
#include "core/application.h"
#include "processing/adaptor.h"
#include "processing/pipeline.h"

#include <vtkSmartPointer.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>
#include <vtkUnstructuredGrid.h>

#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include <unistd.h>
#include <sys/types.h>


class Inporter
{
public:
  Inporter( Processor& processor
          , const MPIInportSection& section_
          , const std::vector<std::string>& variables_);
  ~Inporter();

  void process( const std::vector<boost::filesystem::path>& newfiles
              , const bool deleteFiles = false);

protected:
  Processor& processor;

  std::vector<boost::filesystem::path> workingFiles;
  std::vector<boost::filesystem::path> completedFiles;

  // inport section: idx of count
  const MPIInportSection section;

  const std::vector<std::string> variables;
  const std::vector<PipelineSpec> pipelines;

  // time
  uint timeStep;
  const double lengthTimeStep;

private:
  void createInporters( Descriptor& descriptor, const boost::filesystem::path& filename
                      , std::vector<std::unique_ptr<Adaptor>>& outInporters);
};


class RawNetCDFDataFileInporter : public Adaptor
{
public:
  static bool canProcess(const boost::filesystem::path& file);

public:
  RawNetCDFDataFileInporter( Descriptor& descriptor
                           , const std::string& name);

  void process(const boost::filesystem::path& file) override;

protected:
  vtkSmartPointer<vtkImageData> processRawNetCDFDataFile(
      const boost::filesystem::path& filepath
    , const std::string& varname
    , int global_extent_out[6]);
};

class XMLImageDataFileInporter : public Adaptor
{
public:
  static bool canProcess(const boost::filesystem::path& file);

public:
  XMLImageDataFileInporter( Descriptor& descriptor
                          , const std::string& name);

  void process(const boost::filesystem::path& file) override;

protected:
  vtkSmartPointer<vtkImageData> processXMLImageDataFile(
      const boost::filesystem::path& filepath
    , const std::string& varname
    , int global_extent_out[6]);
};


#endif
