#ifndef APPLICATION_HEADER
#define APPLICATION_HEADER

#include "options.h"

#include <vector>
#include <set>

#include <boost/filesystem.hpp>

#include <unistd.h>
#include <sys/types.h>

#include <vtkNew.h>
#include <vtkMPICommunicator.h>


class Notify;
class vtkMPICommunicatorOpaqueComm;


class MPIApplication
{
public:
  MPIApplication(int* argc, char** argv[]);
  ~MPIApplication();

  static const int ROOT_RANK = 0;

  bool isRoot() const { return rank == ROOT_RANK; }
  int getRank() const { return rank; }

  int getSize() const { return size; }

  bool hasFiles(const std::vector<boost::filesystem::path>& newfiles) const;
  void collectFiles(std::vector<boost::filesystem::path>& inout_newfiles);

  bool isDone(const Notify& notify);

protected:
  int rank;
  int size;
};


class MPICatalystApplication : public MPIApplication
{
public:
  MPICatalystApplication(int* argc, char** argv[]);
  ~MPICatalystApplication();

  bool isNotifier() const { notifier; }
  bool isInporter() const { return getInporterIndex() >= 0; }

  const Configuration& getConfigs() const { return configs; }

  const std::pair<int, size_t>& getInporterSection() const { return inporterSection; };
  int getInporterIndex() const { return inporterSection.first; };
  size_t getInporterCount() const { return inporterSection.second; };

  vtkMPICommunicatorOpaqueComm& getCommunicator();

protected:
  Configuration configs;
  vtkNew<vtkMPICommunicator> communicator;
  bool notifier;
  std::pair<int, size_t> inporterSection;
};

#endif
