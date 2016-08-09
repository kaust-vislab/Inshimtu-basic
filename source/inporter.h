#ifndef INPORTER_HEADER
#define INPORTER_HEADER

#include "adaptor.h"

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
  Inporter(Processor& processor, const std::vector<std::string>& variables);
  ~Inporter();

  void process(const std::vector<boost::filesystem::path>& newfiles);

protected:

  Processor& processor;

  std::vector<boost::filesystem::path> workingFiles;
  std::vector<boost::filesystem::path> completedFiles;

  const std::vector<std::string> variables;

  // time
  uint timeStep;
  const uint maxTimeSteps;
  const double lengthTimeStep;
};

class RawNetCDFDataFileInporter : public Adaptor
{
public:
  RawNetCDFDataFileInporter( Processor& processor
                           , const std::vector<std::string>& names
                           , uint timeStep, double time, bool forceOutput);

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
  XMLImageDataFileInporter( Processor& processor
                          , const std::vector<std::string>& names
                          , uint timeStep, double time, bool forceOutput);

  void process(const boost::filesystem::path& file) override;

protected:
  vtkSmartPointer<vtkImageData> processXMLImageDataFile(
      const boost::filesystem::path& filepath
    , const std::string& varname
    , int global_extent_out[6]);
};


#endif
