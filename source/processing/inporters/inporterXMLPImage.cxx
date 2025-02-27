/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#include "processing/inporters/inporterXMLPImage.h"
#include "utils/logger.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkImageData.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkDataReader.h>
#include <vtkDataSetReader.h>
#include <vtkXMLReader.h>
#include <vtkXMLPImageDataReader.h>
#include <vtkMPASReader.h>
#include <vtkNetCDFReader.h>
#include <vtkNetCDFCFReader.h>
#include <vtkInformation.h>
#include <vtkIndent.h>

#include <vtk_netcdf.h>

#include <memory>
#include <iostream>
#include <algorithm>

#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/types.h>

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>


namespace fs = boost::filesystem;


XMLPImageDataFileInporter::XMLPImageDataFileInporter(
    Descriptor& descriptor
  , const std::string& name)
  : Adaptor(descriptor, name)
{
    BOOST_LOG_TRIVIAL(trace) << "XMLPImageDataFileInporter created";
}

bool XMLPImageDataFileInporter::canProcess(const boost::filesystem::path& file)
{
  return vtkNew<vtkXMLPImageDataReader>()->CanReadFile(file.c_str());
}

void XMLPImageDataFileInporter::process(const boost::filesystem::path& file)
{
  if (doesRequireProcessing() && canProcess(file))
  {
    int global_extent[6];
    vtkSmartPointer<vtkDataObject> data;

    data = processXMLPImageDataFile(file, name, global_extent);

    if (data)
    {
      coprocess(data, global_extent);
    }
    else
    {
      BOOST_LOG_TRIVIAL(warning) << "WARNING: could not process input file '"
                                 << file << "'";
    }
  }
}

vtkSmartPointer<vtkImageData> XMLPImageDataFileInporter::processXMLPImageDataFile(
    const fs::path& filepath
  , const std::string& //varname
  , int global_extent_out[6])
{
  vtkNew<vtkXMLPImageDataReader> reader;

  BOOST_LOG_TRIVIAL(info) << "Processing XMLPImage Datafile:'" << filepath << "'";

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkImageData> data = reader->GetOutput();
  {
    int *local_extent = data->GetExtent();
    for (int i=0; i<6; ++i)
      global_extent_out[i] = local_extent[i];
  }

  return data;
}
