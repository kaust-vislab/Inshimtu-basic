/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#ifndef PROCESSING_INPORTER_HEADER
#define PROCESSING_INPORTER_HEADER

#include "core/options.h"
#include "core/application.h"
#include "processing/adaptor.h"
#include "processing/pipeline.h"

//#include <vtkSmartPointer.h>
//#include <vtkDataObject.h>
//#include <vtkImageData.h>
//#include <vtkUnstructuredGrid.h>

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
          , const std::vector<PipelineSpec>& pipelines_);
  ~Inporter();

  void process( const std::vector<boost::filesystem::path>& newfiles
              , const bool deleteFiles = false);

protected:
  // inport section: idx of count
  const MPIInportSection section;

  const std::vector<PipelineSpec> pipelines;

  std::vector<boost::filesystem::path> availableFiles;

  std::vector<boost::filesystem::path> workingFiles;
  std::vector<boost::filesystem::path> completedFiles;

  Processor& processor;

  // time
  uint timeStep;
  const double lengthTimeStep;

private:
  void createTasks( double time, bool forceOutput
                  , std::vector<std::unique_ptr<TaskState>>& outTasks);
};


#endif
