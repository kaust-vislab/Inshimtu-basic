/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */#ifndef CORE_APPLICATION_HEADER
#define CORE_APPLICATION_HEADER

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

class MPISection
{
public:

  typedef int NodeRank;
  typedef size_t SectionIndex;
  typedef size_t SectionSize;

  MPISection(NodeRank rank_, SectionIndex index_, SectionSize size_);

  NodeRank getRank() const { return rank; }
  SectionIndex getIndex() const { return index; }
  SectionSize getSize() const { return size; }

private:
  NodeRank rank;
  SectionIndex index;
  SectionSize size;
};

class MPIInportSection : public MPISection
{
public:

  static const MPISection::SectionIndex ROOT_INDEX = 0;

  MPIInportSection(NodeRank rank, SectionIndex index, SectionSize sz);
};


class MPIApplication
{
public:

  static const MPISection::NodeRank ROOT_RANK = 0;

  MPIApplication(int* argc, char** argv[]);
  virtual ~MPIApplication();

  bool isRoot() const { return appSection->getRank() == ROOT_RANK; }
  MPISection::NodeRank getRank() const { return appSection->getRank(); }

  MPISection::SectionSize getSize() const { return appSection->getSize(); }

protected:
  std::unique_ptr<MPISection> appSection;
};


class MPICatalystApplication : public MPIApplication
{
public:

  MPICatalystApplication(int* argc, char** argv[]);
  virtual ~MPICatalystApplication();

  bool isNotifier() const { return notifier; }
  bool isInporter() const { return getInporterNode() >= 0; }

  const Configuration& getConfigs() const { return configs; }

  const MPIInportSection& getInporterSection() const;
  MPISection::NodeRank getInporterNode() const { return inporterSection->getRank(); }
  size_t getInporterIndex() const { return inporterSection->getIndex(); }

  vtkMPICommunicatorOpaqueComm& getInporterCommunicator();
  vtkMPICommunicatorOpaqueComm& getCoordinationCommunicator();

protected:
  Configuration configs;
  vtkNew<vtkMPICommunicator> coordCommunicator;
  vtkNew<vtkMPICommunicator> inportCommunicator;
  bool notifier;
  std::unique_ptr<MPIInportSection> inporterSection;
};

#endif
