#ifndef ADAPTOR_HEADER
#define ADAPTOR_HEADER

#include <vtkNew.h>
#include <vtkCPProcessor.h>

#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include <sys/types.h>

class Attributes;
class Grid;
class vtkDataObject;
class vtkCommunicatorOpaqueComm;

class Catalyst
{
public:
  Catalyst( vtkMPICommunicatorOpaqueComm& communicator
          , const std::vector<boost::filesystem::path>& scripts
          , uint delay = 0);
  ~Catalyst();

  void coprocess( vtkDataObject* data
                , double time, uint timeStep, bool forceOutput);

private:
  vtkNew<vtkCPProcessor> processor;
};

#endif
