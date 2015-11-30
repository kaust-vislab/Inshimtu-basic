#ifndef APPLICATION_HEADER
#define APPLICATION_HEADER

#include <unistd.h>
#include <sys/types.h>

class MPIApplication
{
public:
  MPIApplication(int* argc, char** argv[]);
  ~MPIApplication();
};

#endif
