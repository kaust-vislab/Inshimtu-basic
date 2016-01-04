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


namespace fs = boost::filesystem;


Inporter::Inporter(Catalyst& coprocessor)
  : coprocessor(coprocessor)
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
        // new file is new
        std::cout << "New working file: '" << name << "'" << std::endl;

        workingFiles.push_back(name);

        double time = timeStep * lengthTimeStep;

        std::cout << "Inporter: Updating data and Catalyst..." << std::endl;

        vtkSmartPointer<vtkImageData> data = processXMLImageDataFile(name);

        if (vtkMPASReader::CanReadFile(name.c_str()))
        {
          vtkSmartPointer<vtkUnstructuredGrid> datagrid = processMPASDataFile(name);
        }
        else if (vtkNetCDFCFReader::CanReadFile(name.c_str()))
        {
          vtkSmartPointer<vtkDataObject> dataobj2 = processNetCDFCFDataFile(name);
        }
        else
        {
          // TODO: test if filepath is an NC file?
          //vtkSmartPointer<vtkDataObject> data1 = processNetCDFDataFile(name);
          //vtkSmartPointer<vtkImageData> data2 = processRawNetCDFDataFile(name);
        }

        completedFiles.push_back(name);

        //attributes.UpdateFields(time);
        std::cout << "\t\t...Done UpdateFields" << std::endl;

        coprocessor.coprocess(data.Get(), time, timeStep, timeStep >= maxTimeSteps-1);

        ++timeStep;
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

void Inporter::processDataFile(const fs::path& filepath)
{
  vtkNew<vtkDataReader> reader;
  vtkNew<vtkInformation> metadata;

  std::cout << "Processing DataFile:'" << filepath << "'" << std::endl;

  // TODO: handle read result errors
  reader->SetFileName(filepath.c_str());
  bool ok = reader->OpenVTKFile();

  if (!ok)
  {
    std::cerr << "Invalid DataFile:'" << filepath << "'" << std::endl;
    return;
  }

  ok = ok && reader->ReadHeader();

  if (!ok)
  {
    std::cerr << "Invalid DataFile:'" << filepath << "'" << std::endl;
    return;
  }

  ok = ok && reader->ReadMetaData(metadata.Get());

  if (!ok)
  {
    std::cerr << "Invalid DataFile:'" << filepath << "'" << std::endl;
    return;
  }

  const int numScalars = reader->GetNumberOfScalarsInFile();
  const int numVectors = reader->GetNumberOfVectorsInFile();
  const int numTensors = reader->GetNumberOfTensorsInFile();
  const int numNormals = reader->GetNumberOfNormalsInFile();
  const int numCoords = reader->GetNumberOfTCoordsInFile();
  const int numFields = reader->GetNumberOfFieldDataInFile();

  std::cout << "numScalars:" << numScalars << std::endl;
  for(int i=0;i<numScalars;++i)
  {
    std::cout << "  " << reader->GetScalarsNameInFile(i) << std::endl;
  }

  std::cout << "numVectors:" << numVectors << std::endl;
  for(int i=0;i<numVectors;++i)
  {
    std::cout << "  " << reader->GetVectorsNameInFile(i) << std::endl;
  }

  std::cout << "numTensors:" << numTensors << std::endl;
  for(int i=0;i<numTensors;++i)
  {
    std::cout << "  " << reader->GetTensorsNameInFile(i) << std::endl;
  }

  std::cout << "numNormals:" << numNormals << std::endl;
  for(int i=0;i<numNormals;++i)
  {
    std::cout << "  " << reader->GetNormalsNameInFile(i) << std::endl;
  }

  std::cout << "numCoords:" << numCoords << std::endl;
  for(int i=0;i<numCoords;++i)
  {
    std::cout << "  " << reader->GetTCoordsNameInFile(i) << std::endl;
  }

  std::cout << "numFields:" << numFields << std::endl;
  for(int i=0;i<numFields;++i)
  {
    std::cout << "  " << reader->GetFieldDataNameInFile(i) << std::endl;
  }

  metadata->PrintSelf(std::cout, vtkIndent());

  reader->CloseVTKFile();
}

vtkSmartPointer<vtkImageData> Inporter::processXMLImageDataFile(const fs::path& filepath)
{
  vtkNew<vtkXMLImageDataReader> reader;
  vtkNew<vtkInformation> metadata;

  std::cout << "Processing XMLImage DataFile:'" << filepath << "'" << std::endl;

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkImageData> data = reader->GetOutput();

  reader->PrintSelf(std::cout, vtkIndent());

  return data;
}

vtkSmartPointer<vtkUnstructuredGrid> Inporter::processMPASDataFile(const fs::path& filepath)
{
  vtkNew<vtkMPASReader> reader;

  std::cout << "Processing MPAS NetCDF DataFile:'" << filepath << "'" << std::endl;

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkUnstructuredGrid> data = reader->GetOutput();

  reader->PrintSelf(std::cout, vtkIndent());

  return data;
}

vtkSmartPointer<vtkDataObject> Inporter::processNetCDFCFDataFile(const fs::path& filepath)
{
  vtkNew<vtkNetCDFCFReader> reader;

  std::cout << "Processing NetCDF DataFile:'" << filepath << "'" << std::endl;

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkDataObject> data = reader->GetOutput();

  reader->PrintSelf(std::cout, vtkIndent());

  return data;
}

vtkSmartPointer<vtkDataObject> Inporter::processNetCDFDataFile(const fs::path& filepath)
{
  vtkNew<vtkNetCDFReader> reader;

  std::cout << "Processing NetCDF DataFile:'" << filepath << "'" << std::endl;

  reader->SetFileName(filepath.c_str());
  reader->Update();

  vtkSmartPointer<vtkDataObject> data = reader->GetOutput();

  reader->PrintSelf(std::cout, vtkIndent());

  return data;
}

vtkSmartPointer<vtkImageData> Inporter::processRawNetCDFDataFile(const fs::path& filepath)
{
// https://www.unidata.ucar.edu/software/netcdf/docs/simple_xy_nc4_rd_8c-example.html
// https://www.unidata.ucar.edu/software/netcdf/docs/pres_temp_4D_rd_8c-example.html

  std::cout << "Processing NetCDF DataFile:'" << filepath << "'" << std::endl;

  // TODO: read header to get data variables and dimensions; dynamically allocate correct vtk*Data type.
  const int NX = 10;
  const int NY = 6;

  vtkSmartPointer<vtkImageData> data = vtkImageData::New();
  data->SetDimensions(NX, NY, 0);
  data->PrepareForNewData();

  // netCDF ID for the file and data variable.
  int ncid, varid;

  // Error handling
  int retval;

  // Open the file. NC_NOWRITE tells netCDF we want read-only access to the file.
  retval = nc_open(filepath.c_str(), NC_NOWRITE, &ncid);
  assert(retval == 0);

  // Get the varid of the data variable, based on its name.
  retval = nc_inq_varid(ncid, "data", &varid);
  assert(retval == 0);

  // Read the data.
  retval = nc_get_var_int(ncid, varid, static_cast<int*>(data->GetScalarPointer()));
  // retval = nc_get_var_float(ncid, varid, &data[0][0])
  assert(retval == 0);

  // Close the file, freeing all resources.
  retval = nc_close(ncid);
  assert(retval == 0);

  return data;
}

vtkSmartPointer<vtkImageData> Inporter::processHDF5DataFile(const fs::path& filepath)
{

  std::cout << "Processing HDF5 DataFile:'" << filepath << "'" << std::endl;

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


