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

namespace FEAdaptor
{
  void Initialize(int numScripts, const char* scripts[]);

  void Finalize();

  void CoProcess( const Grid& grid, const Attributes& attributes, double time
                , uint timeStep, bool forceOutput);
}

class Catalyst
{
public:
  Catalyst( const std::vector<boost::filesystem::path>& scripts
          , uint delay = 5);
  ~Catalyst();

  void coprocess( vtkDataObject* data
                , double time, uint timeStep, bool forceOutput);

private:
  vtkNew<vtkCPProcessor> processor;
};

#endif
