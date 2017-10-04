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

  typedef int NodeRank;
  typedef std::pair<NodeRank, size_t> InporterSection;

  Descriptor( Processor& processor
            , const InporterSection& section_
            , uint timeStep, double time, bool forceOutput);
  virtual ~Descriptor();

  bool doesRequireProcessing() const;

  const InporterSection& getSection() const { return section; }

private:
  Processor& processor;

protected:
friend class Adaptor;

  vtkNew<vtkCPDataDescription> description;
  bool requireProcessing;

  // inport section: idx of count
  const InporterSection section;
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

  typedef std::pair<size_t, size_t> Extent;

  bool doesRequireProcessing() const;
  Extent getExtent(size_t max) const;
  const Descriptor::InporterSection& getSection() const;
  void coprocess(vtkDataObject* data, int global_extent[6]);

protected:
  std::string name;
};


#endif
