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
friend class Descriptor;
  vtkNew<vtkCPProcessor> processor;
};

class Descriptor
{
public:
  Descriptor( Processor& processor
            , uint timeStep, double time, bool forceOutput);
  virtual ~Descriptor();

  bool doesRequireProcessing() const;

private:
  Processor& processor;

protected:
friend class Adaptor;
  vtkNew<vtkCPDataDescription> description;
  bool requireProcessing;
};


class Adaptor
{
public:
  Adaptor(Descriptor& descriptor, const std::string& name);
  virtual ~Adaptor();

  virtual void process(const boost::filesystem::path& file) = 0;

private:
  Descriptor& descriptor;

protected:
  bool doesRequireProcessing() const;
  void coprocess(vtkDataObject* data, int global_extent[6]);

protected:
  std::string name;
};


#endif
