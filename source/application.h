#ifndef APPLICATION_HEADER
#define APPLICATION_HEADER

#include "options.h"

#include <vector>
#include <map>
#include <set>

#include <boost/filesystem.hpp>

#include <unistd.h>
#include <sys/types.h>

#include <vtkNew.h>
#include <vtkMPICommunicator.h>


class vtkMPICommunicatorOpaqueComm;


class MPIApplication
{
public:
  MPIApplication(int* argc, char** argv[]);
  virtual ~MPIApplication();

  typedef int NodeRank;

  static const NodeRank ROOT_RANK = 0;

  bool isRoot() const { return rank == ROOT_RANK; }
  NodeRank getRank() const { return rank; }

  int getSize() const { return size; }

protected:
  NodeRank rank;
  int size;
};


class MPICatalystApplication : public MPIApplication
{
public:

  typedef std::pair<NodeRank, size_t> InporterSection;

  MPICatalystApplication(int* argc, char** argv[]);
  virtual ~MPICatalystApplication();

  bool isNotifier() const { notifier; }
  bool isInporter() const { return getInporterIndex() >= 0; }

  const Configuration& getConfigs() const { return configs; }

  const InporterSection& getInporterSection() const { return inporterSection; };
  NodeRank getInporterIndex() const { return inporterSection.first; };
  size_t getInporterCount() const { return inporterSection.second; };

  vtkMPICommunicatorOpaqueComm& getCommunicator();

protected:
  Configuration configs;
  vtkNew<vtkMPICommunicator> communicator;
  bool notifier;
  InporterSection inporterSection;
};

#endif
