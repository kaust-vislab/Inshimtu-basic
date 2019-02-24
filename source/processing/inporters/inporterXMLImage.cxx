/* Inshimtu - An In-situ visualization co-processing shim
 *
 * Copyright 2015-2019, KAUST
 * Licensed under GPL3 -- see LICENSE.txt
 */

#include "processing/inporters/inporterXMLImage.h"
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

#include <vtk_netcdfcpp.h>
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


XMLImageDataFileInporter::XMLImageDataFileInporter(
    Descriptor& descriptor
  , const std::string& name)
  : Adaptor(descriptor, name)
{
}

bool XMLImageDataFileInporter::canProcess(const boost::filesystem::path& file)
{
  return vtkNew<vtkXMLImageDataReader>()->CanReadFile(file.c_str());
}

void XMLImageDataFileInporter::process(const boost::filesystem::path& file)
{
  if (doesRequireProcessing() && canProcess(file))
  {
    int global_extent[6];
    vtkSmartPointer<vtkDataObject> data;

    data = processXMLImageDataFile(file, name, global_extent);

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

vtkSmartPointer<vtkImageData> XMLImageDataFileInporter::processXMLImageDataFile(
    const fs::path& filepath
  , const std::string& //varname
  , int global_extent_out[6])
{
  vtkNew<vtkXMLImageDataReader> reader;

  BOOST_LOG_TRIVIAL(info) << "Processing XMLImage Datafile:'" << filepath << "'";

  reader->SetFileName(filepath.c_str());
  reader->Update();

  // TODO: handle multiple inporters (or force single inporter to manage)
  vtkSmartPointer<vtkImageData> data = reader->GetOutput();

  // TODO: acquire accurate global_extent
  {
    int *local_extent = data->GetExtent();
    for (int i=0; i<6; ++i)
      global_extent_out[i] = local_extent[i];
  }

  {
    std::stringstream ss;
    reader->PrintSelf(ss, vtkIndent());
    BOOST_LOG_TRIVIAL(debug) << ss;
  }

  return data;
}


// TODO: Untested
/*

vtkSmartPointer<vtkImageData> Inporter::processHDF5DataFile(const fs::path& filepath)
{

  BOOST_LOG_TRIVIAL(info) << "Processing HDF5 Datafile:'" << filepath << "'";

  vtkSmartPointer<vtkImageData> image;

  hid_t       file, dataset;
  hid_t       datatype, dataspace;
  hid_t       memspace;
  H5T_class_t hclass;
  H5T_order_t order;
  size_t      data_elem_size;

  hsize_t     dimsm[3];     // memory space dimensions
  hsize_t     dims_out[2];  // dataset dimensions
  herr_t      status;

  // TODO: read header to get data variables and dimensions; dynamically allocate correct vtk*Data type.
  const int NX = 10;
  const int NY = 6;
  const int NZ = 3;
  const int NX_SUB = 2;
  const int NY_SUB = 2;
  const int RANK_OUT = 3;
  int data_out[NX][NY][NZ ];  // output buffer

  hsize_t   count[2];      // size of the hyperslab in the file
  hsize_t   offset[2];     // hyperslab offset in the file
  hsize_t   count_out[3];  // size of the hyperslab in memory
  hsize_t   offset_out[3]; // hyperslab offset in memory
  size_t    sz;
  int       i, j, k, status_n, rank;

  file = H5Fopen(filepath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  assert(file >= 0);

  dataset = H5Dopen(file, "dataset", H5P_DEFAULT);
  assert(dataset >= 0);

  datatype = H5Dget_type(dataset);
  assert(datatype >= 0);

  hclass = H5Tget_class(datatype);
  order = H5Tget_order(datatype);
  sz  = H5Tget_size(datatype);

  if (hclass == H5T_INTEGER)
    BOOST_LOG_TRIVIAL(debug) << "Data set has INTEGER type";

  if (order == H5T_ORDER_LE)
    BOOST_LOG_TRIVIAL(debug) << "Little endian order";

  BOOST_LOG_TRIVIAL(debug) << "Data size: " << sz;

  dataspace = H5Dget_space(dataset);
  assert(dataspace >= 0);

  rank = H5Sget_simple_extent_ndims(dataspace);
  status_n = H5Sget_simple_extent_dims(dataspace, dims_out, nullptr);

  BOOST_LOG_TRIVIAL(debug) << "rank: " << rank << ", dimensions: "
            << static_cast<unsigned long>(dims_out[0]) << " x,"
            << static_cast<unsigned long>(dims_out[1]) << " y";

  // Define hyperslab in the dataset.
  offset[0] = 1;
  offset[1] = 2;
  count[0]  = NX_SUB;
  count[1]  = NY_SUB;
  status = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, nullptr, count, nullptr);

  // Define the memory dataspace.
  dimsm[0] = NX;
  dimsm[1] = NY;
  dimsm[2] = NZ ;
  memspace = H5Screate_simple(RANK_OUT, dimsm, nullptr);
  assert(memspace >= 0);

  // Define memory hyperslab.
  offset_out[0] = 3;
  offset_out[1] = 0;
  offset_out[2] = 0;
  count_out[0]  = NX_SUB;
  count_out[1]  = NY_SUB;
  count_out[2]  = 1;
  status = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, offset_out, nullptr, count_out, nullptr);

  // Read data from hyperslab in the file into the hyperslab in memory and display.
  status = H5Dread(dataset, H5T_NATIVE_INT, memspace, dataspace, H5P_DEFAULT, data_out);

  H5Tclose(datatype);
  H5Dclose(dataset);
  H5Sclose(dataspace);
  H5Sclose(memspace);
  H5Fclose(file);

  return image;
}

*/
