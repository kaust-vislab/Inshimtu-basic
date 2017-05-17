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

// TODO:<vtk_netcdfcpp.h> points to wrong directory on ParaView v5.1.2
//#include <vtk_netcdfcpp.h>
#include <vtknetcdf/include/netcdfcpp.h>

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

Inporter::Inporter( Processor& processor_
                  , const std::pair<int, size_t>& section_
                  , const std::vector<std::string>& variables_)
  : processor(processor_)
  , variables(variables_)
  , section(section_)
  , timeStep(0)
  , lengthTimeStep(1.0)
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
        const bool forceOutput = false;
        Descriptor descriptor(processor, section, timeStep, time, forceOutput);
        std::vector<std::unique_ptr<Adaptor>> inporters;

        // Create inporters
        createInporters(descriptor, name, inporters);

        // new file is new
        std::cout << "New working file: '" << name << "'" << std::endl;

        workingFiles.push_back(name);

        std::cout << "Inporter: Updating data and Catalyst..." << std::endl;

        // Inport file
        for (auto& inporter : inporters)
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

void Inporter::createInporters( Descriptor& descriptor, const fs::path& filename
                              , std::vector<std::unique_ptr<Adaptor>>& outInporters)
{
  for (const auto& vsets : variables)
  {
    if (RawNetCDFDataFileInporter::canProcess(filename))
    {
      std::cout << "Creating RawNetCDFDataFileInporter for: '" << vsets << "'" << std::endl;

      std::vector<std::string> vars;
      boost::split(vars, vsets, boost::is_any_of(","), boost::token_compress_on);

      for (const auto& v : vars)
      {
        outInporters.push_back(
            std::unique_ptr<Adaptor>(
                new RawNetCDFDataFileInporter(descriptor, v)));
      }
    }
    else if (XMLImageDataFileInporter::canProcess(filename))
    {
      std::cout << "Creating XMLImageDataFileInporter for: '" << vsets << "'" << std::endl;

      outInporters.push_back(
          std::unique_ptr<Adaptor>(
              new XMLImageDataFileInporter(descriptor, vsets)));
    }
  }
}



RawNetCDFDataFileInporter::RawNetCDFDataFileInporter(
    Descriptor& descriptor
  , const std::string& name)
  : Adaptor(descriptor, name)
{
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
      std::cerr << "WARNING: could not process variable '" << name
                << "' from input file '" << file << "'" << std::endl;
    }
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

  const std::pair<size_t, size_t> extentZ = getExtent(lenZ);

  int local_offset[3] = {0, 0, static_cast<int>(extentZ.first)};
  int local_size[3] = {static_cast<int>(lenX), static_cast<int>(lenY), static_cast<int>(extentZ.second)};
  int local_extent[6] = { local_offset[0], local_offset[0]+local_size[0]-1
                        , local_offset[1], local_offset[1]+local_size[1]-1
                        , local_offset[2], local_offset[2]+local_size[2]-1
                        };


  vtkSmartPointer<vtkImageData> data = vtkImageData::New();
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
  std::cout << "Processing variable '" << varname
            << "' from NetCDF Datafile:'" << filepath << "' with netcdf library"
            << std::endl;

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

  vtkSmartPointer<vtkImageData> data = vtkImageData::New();
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

  // TODO: handle multiple inporters (or force single inporter to manage)
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
