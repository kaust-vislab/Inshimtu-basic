#ifndef INPORTER_HEADER
#define INPORTER_HEADER

#include <vtkSmartPointer.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>
#include <vtkUnstructuredGrid.h>

#include <vector>
#include <string>

#include <unistd.h>
#include <sys/types.h>

class Catalyst;

class Inporter
{
public:
  Inporter(Catalyst& coprocessor);
  ~Inporter();

  void process(const std::vector<std::string>& newfiles);

protected:

  Catalyst& coprocessor;

  std::vector<std::string> workingFiles;
  std::vector<std::string> completedFiles;

  // time
  uint timeStep;
  const uint maxTimeSteps;
  const double lengthTimeStep;

private:

  void processDataFile(const std::string& filepath);

  vtkSmartPointer<vtkImageData> processXMLImageDataFile(const std::string& filepath);
  vtkSmartPointer<vtkUnstructuredGrid> processMPASDataFile(const std::string& filepath);
  vtkSmartPointer<vtkDataObject> processNetCDFCFDataFile(const std::string& filepath);
  vtkSmartPointer<vtkDataObject> processNetCDFDataFile(const std::string& filepath);
  vtkSmartPointer<vtkImageData> processRawNetCDFDataFile(const std::string& filepath);
  vtkSmartPointer<vtkImageData> processHDF5DataFile(const std::string& filepath);

};

#endif
