#ifndef APPLICATION_HEADER
#define APPLICATION_HEADER

#include <vector>

#include <boost/filesystem.hpp>

#include <unistd.h>
#include <sys/types.h>

#include <vtkNew.h>
#include <vtkMPICommunicator.h>


class INotify;
class vtkMPICommunicatorOpaqueComm;


class MPIApplication
{
public:
  MPIApplication(int* argc, char** argv[]);
  ~MPIApplication();

  static const int ROOT_RANK = 0;

  bool isRoot() const { return rank == 0; }
  int getRank() const { return rank; }

  int getSize() const { return size; }

  bool hasFiles(const std::vector<boost::filesystem::path>& newfiles) const;
  void collectFiles(std::vector<boost::filesystem::path>& newfiles);

  bool isDone(const INotify& notify);

protected:
  int rank;
  int size;
};


class MPICatalystApplication : public MPIApplication
{
public:
  MPICatalystApplication(int* argc, char** argv[]);
  ~MPICatalystApplication();

  bool isInporter() const { return rank == 0; }

  vtkMPICommunicatorOpaqueComm& getCommunicator();

protected:
  vtkNew<vtkMPICommunicator> communicator;
};

#endif
