#ifndef ADAPTOR_HEADER
#define ADAPTOR_HEADER

#include <vtkNew.h>
#include <vtkCPProcessor.h>

#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include <sys/types.h>

class vtkDataObject;
class vtkCommunicatorOpaqueComm;


class Processor
{
public:
  Processor( vtkMPICommunicatorOpaqueComm& communicator
           , const std::vector<boost::filesystem::path>& scripts
           , uint delay = 0);
  ~Processor();

private:
friend class Adaptor;
  vtkNew<vtkCPProcessor> processor;
};


class Adaptor
{
public:
  Adaptor( Processor& processor
         , const std::vector<std::string>& names
         , uint timeStep, double time, bool forceOutput);
  virtual ~Adaptor();

  bool doesRequireProcessing() const;

  virtual void process(const boost::filesystem::path& file) = 0;

private:
  Processor& processor;

protected:
  void setData(vtkDataObject* data, const std::string& name, int global_extent[6]);
  void coprocess();

protected:
  std::vector<std::string> names;
  vtkNew<vtkCPDataDescription> description;
  bool requireProcessing;
};


#endif
