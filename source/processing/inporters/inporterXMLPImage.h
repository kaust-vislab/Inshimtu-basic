/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#ifndef PROCESSING_INPORTERS_XMLPIMAGE_HEADER
#define PROCESSING_INPORTERS_XMLPIMAGE_HEADER

#include "core/options.h"
#include "core/application.h"
#include "processing/adaptorV2.h"
#include "processing/pipeline.h"

#include <vtkSmartPointer.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>
#include <vtkUnstructuredGrid.h>

#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include <unistd.h>
#include <sys/types.h>


class XMLPImageDataFileInporter : public Adaptor
{
public:
  static bool canProcess(const boost::filesystem::path& file);

public:
  XMLPImageDataFileInporter( Descriptor& descriptor
                          , const std::string& name);

  void process(const boost::filesystem::path& file) override;

protected:
  vtkSmartPointer<vtkImageData> processXMLPImageDataFile(
      const boost::filesystem::path& filepath
    , const std::string& varname
    , int global_extent_out[6]);
};


#endif
