#include "inporter.h"
#include "adaptor.h"
#include "logger.h"

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

#include <hdf5.h>

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


Inporter::Inporter( Processor& processor_
                  , const std::vector<std::string>& variables_)
  : processor(processor_)
  , variables(variables_)
  , timeStep(0)
  , maxTimeSteps(100)
  , lengthTimeStep(0.01)
{
  std::cout << "STARTED Inporter" << std::endl;
}

Inporter::~Inporter()
{
  std::cout << "FINISHED Inporter. Time:" << timeStep << std::endl;
}

void Inporter::process(const std::vector<fs::path>& newfiles)
{
  std::vector<fs::path> processFiles;

  for(const fs::path& name : newfiles)
  {
    auto wfitr = std::find(workingFiles.begin(), workingFiles.end(), name);
    auto cfitr = std::find(completedFiles.begin(), completedFiles.end(), name);
    if (cfitr != completedFiles.end())
    {
      // expected event
      std::cout << "Completed file: '" << name << "'" << std::endl;

      completedFiles.erase(cfitr);

      assert(wfitr != workingFiles.end() && "Expected file in working set");
    }
    else
    {
      if (wfitr == workingFiles.end())
      {
        const double time = timeStep * lengthTimeStep;
        const bool forceOutput = timeStep >= maxTimeSteps-1;
        std::vector<std::unique_ptr<Adaptor>> inporters;

        // Create inporters
        {
          for (const auto &v : variables)
          {
            std::cout << "Creating RawNetCDFDataFileInporter for: '" << v << "'" << std::endl;

            std::vector<std::string> vars;
            boost::split(vars, v, boost::is_any_of(","), boost::token_compress_on);

            inporters.push_back(std::unique_ptr<Adaptor>(new RawNetCDFDataFileInporter(
                                                 processor
                                               , vars
                                               , timeStep, time, true || forceOutput)));
          }
          // TODO: fix: restore
          /*
          std::cout << "Creating XMLImageDataFileInporter for: '" << "input" << "'" << std::endl;

          inporters.push_back(std::unique_ptr<Adaptor>(new XMLImageDataFileInporter(
                                               processor
                                             , "input"
                                             , timeStep, time, true || forceOutput)));
          */
        }

        // new file is new
        std::cout << "New working file: '" << name << "'" << std::endl;

        workingFiles.push_back(name);

        std::cout << "Inporter: Updating data and Catalyst..." << std::endl;

        // Inport file
        for (auto &inporter : inporters)
        {
          inporter->process(name);
        }

        completedFiles.push_back(name);
        ++timeStep;

        std::cout << "\t\t...Done UpdateFields" << std::endl;
      }
      else
      {
        std::cerr << "WARNING: input file '" << name << "' "
                  << "modified multiple times. "
                  << "Expected source files to be written once, and then closed."
                  << std::endl;
      }
    }
  }
}


RawNetCDFDataFileInporter::RawNetCDFDataFileInporter(
    Processor& processor
  , const std::vector<std::string>& names
  , uint timeStep, double time, bool forceOutput)
  : Adaptor(processor, names, timeStep, time, forceOutput)
{
}

void RawNetCDFDataFileInporter::process(const boost::filesystem::path& file)
{
  if ( doesRequireProcessing()
    && vtkNetCDFCFReader::CanReadFile(file.c_str()))
  {
    int global_extent[6];

    for (const auto& name: names)
    {
      vtkSmartPointer<vtkDataObject> data;

      data = processRawNetCDFDataFile(file, name, global_extent);

      if (data)
      {
        setData(data, name, global_extent);
      }
      else
      {
        std::cerr << "WARNING: could not process variable '" << name
                  << "' from input file '" << file << "'" << std::endl;
      }
    }
    coprocess();
  }
}

vtkSmartPointer<vtkImageData> RawNetCDFDataFileInporter::processRawNetCDFDataFile(
    const fs::path& filepath
  , const std::string& varname
  , int global_extent_out[6])
{
  std::cout << "Processing variable '" << varname
            << "' from NetCDF Datafile:'" << filepath << "' with netcdf library"
            << std::endl;

  int ncid, varid; // netCDF ID for the file and data variable.
  int ndims;
  int dimids[NC_MAX_VAR_DIMS]; // netCDF ID for dimensions.
  nc_type vartype;
  size_t lenX, lenY, lenZ;
  int retval; // Error handling

  // Open the file. NC_NOWRITE tells netCDF we want read-only access to the file.
  retval = nc_open(filepath.c_str(), NC_NOWRITE, &ncid); assert(retval == 0);

  // Get the varid of the data variable, based on its name.
  retval = nc_inq_varid(ncid, varname.c_str(), &varid); assert(retval == 0);

  retval = nc_inq_varndims(ncid, varid, &ndims); assert(retval == 0);
  retval = nc_inq_vardimid(ncid, varid, dimids); assert(retval == 0);
  retval = nc_inq_vartype(ncid, varid, &vartype); assert(retval == 0);

  assert(vartype == NC_FLOAT);
  assert(ndims >= 3);

  retval = nc_inq_dimlen(ncid, dimids[ndims-1], &lenX); assert(retval == 0);
  retval = nc_inq_dimlen(ncid, dimids[ndims-2], &lenY); assert(retval == 0);
  retval = nc_inq_dimlen(ncid, dimids[ndims-3], &lenZ); assert(retval == 0);

  // TODO: find local offset for per-rank files.
  int local_offset[3] = {0, 0, 0};
  int local_size[3] = {static_cast<int>(lenX), static_cast<int>(lenY), static_cast<int>(lenZ)};
  int local_extent[6] = { local_offset[0], local_offset[0]+local_size[0]-1
                        , local_offset[1], local_offset[1]+local_size[1]-1
                        , local_offset[2], local_offset[2]+local_size[2]-1
                        };
  // TODO: acquire accurate global_extent
  {
    for (int i=0; i<6; ++i)
      global_extent_out[i] = local_extent[i];
  }

  vtkSmartPointer<vtkImageData> data = vtkImageData::New();
  data->SetDimensions(local_size);
  data->SetExtent(local_extent);
  data->AllocateScalars(VTK_FLOAT, 1);
  data->GetPointData()->GetScalars()->SetName(varname.c_str());

  // Read the data.
  retval = nc_get_var_float(ncid, varid, static_cast<float*>(data->GetScalarPointer()));
  assert(retval == 0);

  // Close the file, freeing all resources.
  retval = nc_close(ncid);
  assert(retval == 0);

  return data;
}


XMLImageDataFileInporter::XMLImageDataFileInporter(
    Processor& processor
  , const std::vector<std::string>& names
  , uint timeStep, double time, bool forceOutput)
  : Adaptor(processor, names, timeStep, time, forceOutput)
{
}

void XMLImageDataFileInporter::process(const boost::filesystem::path& file)
{
  if ( doesRequireProcessing()
    && vtkNew<vtkXMLImageDataReader>()->CanReadFile(file.c_str()))
  {
    assert(!names.empty());
    const std::string& name(names[0]);

    int global_extent[6];
    vtkSmartPointer<vtkDataObject> data;

    data = processXMLImageDataFile(file, name, global_extent);

    if (data)
    {
      setData(data, name, global_extent);
      coprocess();
    }
    else
    {
      std::cerr << "WARNING: could not process input file '" << file << "'" << std::endl;
    }
  }
}

vtkSmartPointer<vtkImageData> XMLImageDataFileInporter::processXMLImageDataFile(
    const fs::path& filepath
  , const std::string& //varname
  , int global_extent_out[6])
{
  vtkNew<vtkXMLImageDataReader> reader;

  std::cout << "Processing XMLImage Datafile:'" << filepath << "'" << std::endl;

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkImageData> data = reader->GetOutput();

  // TODO: acquire accurate global_extent
  {
    int *local_extent = data->GetExtent();
    for (int i=0; i<6; ++i)
      global_extent_out[i] = local_extent[i];
  }

  reader->PrintSelf(std::cout, vtkIndent());

  return data;
}


// TODO: Untested
/*
         // TODO: don't get the data now, pass in a lamda to delay reading until necessary
        vtkSmartPointer<vtkDataObject> data;

        if (vtkNew<vtkXMLImageDataReader>()->CanReadFile(name.c_str()))
        {
          data = processXMLImageDataFile(name);
        }
        else if (vtkMPASReader::CanReadFile(name.c_str()))
        {
          data = processMPASDataFile(name);
        }
        else if (vtkNetCDFCFReader::CanReadFile(name.c_str()))
        {
          data = processNetCDFCFDataFile(name);
        }
        else if (vtkNetCDFCFReader::CanReadFile(name.c_str()))
        {
          data = processNetCDFDataFile(name);
        }

vtkSmartPointer<vtkUnstructuredGrid> Inporter::processMPASDataFile(const fs::path& filepath)
{
  vtkNew<vtkMPASReader> reader;

  std::cout << "Processing MPAS NetCDF Datafile:'" << filepath << "'" << std::endl;

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkUnstructuredGrid> data = reader->GetOutput();

  reader->PrintSelf(std::cout, vtkIndent());

  return data;
}

vtkSmartPointer<vtkDataObject> Inporter::processNetCDFCFDataFile(const fs::path& filepath)
{
  vtkNew<vtkNetCDFCFReader> reader;

  std::cout << "Processing NetCDF Datafile:'" << filepath << "' with vtkNetCDFCFReader" << std::endl;

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkDataObject> data = reader->GetOutput();

  reader->PrintSelf(std::cout, vtkIndent());

  return data;
}

vtkSmartPointer<vtkDataObject> Inporter::processNetCDFDataFile(const fs::path& filepath)
{
  vtkNew<vtkNetCDFReader> reader;

  std::cout << "Processing NetCDF Datafile:'" << filepath << "' with vtkNetCDFReader" << std::endl;

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkDataObject> data = reader->GetOutput();

  reader->PrintSelf(std::cout, vtkIndent());

  return data;
}


vtkSmartPointer<vtkImageData> Inporter::processHDF5DataFile(const fs::path& filepath)
{

  std::cout << "Processing HDF5 Datafile:'" << filepath << "'" << std::endl;

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
    std::cout << "Data set has INTEGER type" << std::endl;

  if (order == H5T_ORDER_LE)
    std::cout << "Little endian order" << std::endl;

  std::cout << "Data size: " << sz << std::endl;

  dataspace = H5Dget_space(dataset);
  assert(dataspace >= 0);

  rank = H5Sget_simple_extent_ndims(dataspace);
  status_n = H5Sget_simple_extent_dims(dataspace, dims_out, nullptr);

  std::cout << "rank: " << rank << ", dimensions: "
            << static_cast<unsigned long>(dims_out[0]) << " x,"
            << static_cast<unsigned long>(dims_out[1]) << " y" << std::endl;

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
