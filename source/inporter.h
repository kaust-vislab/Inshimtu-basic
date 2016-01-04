#ifndef INPORTER_HEADER
#define INPORTER_HEADER

#include <vtkSmartPointer.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>
#include <vtkUnstructuredGrid.h>

#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include <unistd.h>
#include <sys/types.h>

class Catalyst;

class Inporter
{
public:
  Inporter(Catalyst& coprocessor);
  ~Inporter();

  void process(const std::vector<boost::filesystem::path>& newfiles);

protected:

  Catalyst& coprocessor;

  std::vector<boost::filesystem::path> workingFiles;
  std::vector<boost::filesystem::path> completedFiles;

  // time
  uint timeStep;
  const uint maxTimeSteps;
  const double lengthTimeStep;

private:

  void processDataFile(const boost::filesystem::path& filepath);

  vtkSmartPointer<vtkImageData> processXMLImageDataFile(const boost::filesystem::path& filepath);
  vtkSmartPointer<vtkUnstructuredGrid> processMPASDataFile(const boost::filesystem::path& filepath);
  vtkSmartPointer<vtkDataObject> processNetCDFCFDataFile(const boost::filesystem::path& filepath);
  vtkSmartPointer<vtkDataObject> processNetCDFDataFile(const boost::filesystem::path& filepath);
  vtkSmartPointer<vtkImageData> processRawNetCDFDataFile(const boost::filesystem::path& filepath);
  vtkSmartPointer<vtkImageData> processHDF5DataFile(const boost::filesystem::path& filepath);

};

#endif
