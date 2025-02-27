/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#ifndef PROCESSING_ADAPTOR_HEADER
#define PROCESSING_ADAPTOR_HEADER

#include "core/application.h"

#include <vtkNew.h>

#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include <sys/types.h>
#include <vtkDataObjectToConduit.h>
#include <catalyst.hpp>
#include <catalyst_conduit.hpp>

class vtkDataObject;
class vtkCommunicatorOpaqueComm;


class Processor
{
public:
  Processor( vtkMPICommunicatorOpaqueComm& communicator
           , const Configuration &config);
  ~Processor();

private:
friend class Descriptor;
  conduit_cpp::Node node;
};

class Descriptor
{
public:

  Descriptor( Processor& node
            , const MPIInportSection& section_
            , uint timeStep, double time, bool forceOutput);
  virtual ~Descriptor();

  bool doesRequireProcessing() const;

  const MPIInportSection& getSection() const { return section; }

private:
  Processor& node;

protected:
friend class Adaptor;

  conduit_cpp::Node description;
  vtkSmartPointer<vtkDataObject> descriptor_data;
  bool requireProcessing;

  // inport section: idx of count
  const MPIInportSection section;
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
  const MPIInportSection& getSection() const;
  void coprocess(vtkDataObject* data, int global_extent[6]);

protected:
  std::string name;
};


#endif
