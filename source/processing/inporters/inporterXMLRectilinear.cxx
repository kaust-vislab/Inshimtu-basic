/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#include "processing/inporters/inporterXMLRectilinear.h"
#include "processing/adaptor.h"
#include "utils/logger.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkDataReader.h>
#include <vtkDataSetReader.h>
#include <vtkXMLReader.h>
#include <vtkXMLRectilinearGridReader.h>
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


XMLRectilinearGridFileInporter::XMLRectilinearGridFileInporter(
    Descriptor& descriptor
  , const std::string& name)
  : Adaptor(descriptor, name)
{
    BOOST_LOG_TRIVIAL(trace) << "XMLRectilinearGridFileInporter created";
}

bool XMLRectilinearGridFileInporter::canProcess(const boost::filesystem::path& file)
{
  return vtkNew<vtkXMLRectilinearGridReader>()->CanReadFile(file.c_str());
}

void XMLRectilinearGridFileInporter::process(const boost::filesystem::path& file)
{
  if (doesRequireProcessing() && canProcess(file))
  {
    int global_extent[6];
    vtkSmartPointer<vtkDataObject> data;

    data = processXMLRectilinearGridFile(file, name, global_extent);

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

vtkSmartPointer<vtkRectilinearGrid> XMLRectilinearGridFileInporter::processXMLRectilinearGridFile(
    const fs::path& filepath
  , const std::string& //varname
  , int global_extent_out[6])
{
  vtkNew<vtkXMLRectilinearGridReader> reader;

  BOOST_LOG_TRIVIAL(info) << "Processing XMLRectilinear Grid Datafile:'" << filepath << "'";

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkRectilinearGrid> data = reader->GetOutput();
  {
    int *local_extent = data->GetExtent();
    for (int i=0; i<6; ++i)
      global_extent_out[i] = local_extent[i];
  }

  {
    std::stringstream ss;
    reader->PrintSelf(ss, vtkIndent());
    BOOST_LOG_TRIVIAL(debug) << ss.str();
  }

  return data;
}
