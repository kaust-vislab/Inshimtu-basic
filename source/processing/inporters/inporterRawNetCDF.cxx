/* Inshimtu - An In-situ visualization co-processing shim
 * Licensed under GPL3 -- see LICENSE.txt
 */
#include "processing/inporters/inporterRawNetCDF.h"
#include "processing/adaptor.h"
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
#include <vtkXMLImageDataReader.h>
#include <vtkMPASReader.h>
#include <vtkNetCDFReader.h>
#include <vtkNetCDFCFReader.h>
#include <vtkInformation.h>
#include <vtkIndent.h>

#include <vtk_netcdf.h>

// TODO: Restore when HDF5 functionality is working
//#include <hdf5.h>

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


RawNetCDFDataFileInporter::RawNetCDFDataFileInporter(
    Descriptor& descriptor
  , const std::string& name)
  : Adaptor(descriptor, name)
{
    BOOST_LOG_TRIVIAL(trace) << "RawNetCDFDataFileImporter created";
}

bool RawNetCDFDataFileInporter::canProcess(const boost::filesystem::path& file)
{
  return vtkNetCDFCFReader::CanReadFile(file.c_str());
}

void RawNetCDFDataFileInporter::process(const boost::filesystem::path& file)
{
  if (doesRequireProcessing() && canProcess(file))
  {
    int global_extent[6];

    vtkSmartPointer<vtkDataObject> data;
    data = processRawNetCDFDataFile(file, name, global_extent);

    if (data)
    {
      coprocess(data, global_extent);
    }
    else
    {
      BOOST_LOG_TRIVIAL(warning) << "WARNING: could not process variable '" << name
                                 << "' from input file '" << file << "'";
    }
  }
}

vtkSmartPointer<vtkImageData> RawNetCDFDataFileInporter::processRawNetCDFDataFile(
    const fs::path& filepath
  , const std::string& varname
  , int global_extent_out[6])
{
  BOOST_LOG_TRIVIAL(info) << "Processing variable '" << varname
            << "' on inport node " << getSection().getIndex()
            << " from NetCDF Datafile:'" << filepath << "' with netcdf library";

  int ncid, varid; // netCDF ID for the file and data variable.
  int ndims;
  int dimids[NC_MAX_VAR_DIMS]; // netCDF ID for dimensions.
  nc_type vartype;
  size_t lenX, lenY, lenZ, lenT;
  int retval; // Error handling

  // Open the file. NC_NOWRITE tells netCDF we want read-only access to the file.
  retval = nc_open(filepath.c_str(), NC_NOWRITE, &ncid); assert(retval == NC_NOERR);

  // Get the varid of the data variable, based on its name.
  retval = nc_inq_varid(ncid, varname.c_str(), &varid); assert(retval == NC_NOERR);

  retval = nc_inq_varndims(ncid, varid, &ndims); assert(retval == NC_NOERR);
  retval = nc_inq_vardimid(ncid, varid, dimids); assert(retval == NC_NOERR);
  retval = nc_inq_vartype(ncid, varid, &vartype); assert(retval == NC_NOERR);

  // TODO: Generalize dimension and time support
  
  assert(vartype == NC_FLOAT);
  assert(ndims == 4);

  retval = nc_inq_dimlen(ncid, dimids[ndims-1], &lenX); assert(retval == NC_NOERR);
  retval = nc_inq_dimlen(ncid, dimids[ndims-2], &lenY); assert(retval == NC_NOERR);
  retval = nc_inq_dimlen(ncid, dimids[ndims-3], &lenZ); assert(retval == NC_NOERR);
  retval = nc_inq_dimlen(ncid, dimids[ndims-4], &lenT); assert(retval == NC_NOERR); assert(lenT == 1);

  int global_size[3] = {static_cast<int>(lenX), static_cast<int>(lenY), static_cast<int>(lenZ)};
  int global_extent[6] = { 0, global_size[0]-1
                         , 0, global_size[1]-1
                         , 0, global_size[2]-1
                         };
  {
    for (int i=0; i<6; ++i)
      global_extent_out[i] = global_extent[i];
  }

  const Adaptor::Extent extentZ = getExtent(lenZ);

  int local_offset[3] = {0, 0, static_cast<int>(extentZ.first)};
  int local_size[3] = {static_cast<int>(lenX), static_cast<int>(lenY), static_cast<int>(extentZ.second)};
  int local_extent[6] = { local_offset[0], local_offset[0]+local_size[0]-1
                        , local_offset[1], local_offset[1]+local_size[1]-1
                        , local_offset[2], local_offset[2]+local_size[2]-1
                        };

  vtkSmartPointer<vtkImageData> data = vtkSmartPointer<vtkImageData>::New();
  data->SetDimensions(local_size);
  data->SetExtent(local_extent);
  data->AllocateScalars(VTK_FLOAT, 1);
  data->GetPointData()->GetScalars()->SetName(varname.c_str());


  const size_t vstart[4] = { 0
                           , static_cast<size_t>(local_offset[2])
                           , static_cast<size_t>(local_offset[1])
                           , static_cast<size_t>(local_offset[0])
                           };
  const size_t vcount[4] = { 1
                           , static_cast<size_t>(local_size[2])
                           , static_cast<size_t>(local_size[1])
                           , static_cast<size_t>(local_size[0])
                           };

  // Read the data.
  retval = nc_get_vars_float( ncid, varid, vstart, vcount, nullptr
                            , static_cast<float*>(data->GetScalarPointer()));
  assert(retval == NC_NOERR);

  // Close the file, freeing all resources.
  retval = nc_close(ncid);
  assert(retval == NC_NOERR);

  return data;
}

// TODO: fix linker errors (missing NetCDF functions), test, and replace C netcdf code
/*
vtkSmartPointer<vtkImageData> RawNetCDFDataFileInporter::processRawNetCDFDataFile(
    const fs::path& filepath
  , const std::string& varname
  , int global_extent_out[6])
{
  BOOST_LOG_TRIVIAL(info) << "Processing variable '" << varname
            << "' on inport node " << getSection().getIndex()
            << "' from NetCDF Datafile:'" << filepath << "' with netcdf library";

  NcFile ncfile(filepath.c_str(), NcFile::ReadOnly, nullptr, 0, NcFile::Netcdf4Classic);
  assert(ncfile.is_valid());

  NcToken varid = varname.c_str();
  NcVar* ncvar = ncfile.get_var(varid);
  assert(ncvar != nullptr);
  assert(ncvar->is_valid());
  assert(ncvar->num_dims() == 3);
  assert(ncvar->type() == ncFloat);

  // TODO: find local offset for per-rank files.
  int local_offset[3] = {0, 0, 0};
  size_t* local_sizes = ncvar->edges();
  int local_size[3] = { static_cast<int>(local_sizes[0])
                      , static_cast<int>(local_sizes[1])
                      , static_cast<int>(local_sizes[2])};
  int local_extent[6] = { local_offset[0], local_offset[0]+local_size[0]-1
                        , local_offset[1], local_offset[1]+local_size[1]-1
                        , local_offset[2], local_offset[2]+local_size[2]-1
                        };
  // TODO: acquire accurate global_extent
  {
    for (int i=0; i<6; ++i)
      global_extent_out[i] = local_extent[i];
  }

  vtkSmartPointer<vtkImageData> data = vtkSmartPointer<vtkImageData>::New();
  data->SetDimensions(local_size);
  data->SetExtent(local_extent);
  data->AllocateScalars(VTK_FLOAT, 1);
  data->GetPointData()->GetScalars()->SetName(varname.c_str());

  // Read the data.
  {
    NcBool result;

    result = ncvar->get(static_cast<float*>(data->GetScalarPointer()), local_sizes);
    assert(result);
  }

  return data;
}
*/



// TODO: Untested
/*

vtkSmartPointer<vtkUnstructuredGrid> Inporter::processMPASDataFile(const fs::path& filepath)
{
  vtkNew<vtkMPASReader> reader;

  BOOST_LOG_TRIVIAL(info) << "Processing MPAS NetCDF Datafile:'" << filepath << "'";

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkUnstructuredGrid> data = reader->GetOutput();

  reader->PrintSelf(BOOST_LOG_TRIVIAL(debug), vtkIndent());

  return data;
}

vtkSmartPointer<vtkDataObject> Inporter::processNetCDFCFDataFile(const fs::path& filepath)
{
  vtkNew<vtkNetCDFCFReader> reader;

  BOOST_LOG_TRIVIAL(info) << "Processing NetCDF Datafile:'" << filepath << "' with vtkNetCDFCFReader";

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkDataObject> data = reader->GetOutput();

  reader->PrintSelf(BOOST_LOG_TRIVIAL(debug), vtkIndent());

  return data;
}

vtkSmartPointer<vtkDataObject> Inporter::processNetCDFDataFile(const fs::path& filepath)
{
  vtkNew<vtkNetCDFReader> reader;

  BOOST_LOG_TRIVIAL(info) << "Processing NetCDF Datafile:'" << filepath << "' with vtkNetCDFReader";

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkDataObject> data = reader->GetOutput();

  reader->PrintSelf(BOOST_LOG_TRIVIAL(debug), vtkIndent());

  return data;
}

*/
