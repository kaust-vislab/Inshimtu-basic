cmake_minimum_required(VERSION 3.5.0)
project(Inshimtu)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ParaView 4.3.1 REQUIRED COMPONENTS
  vtkPVPythonCatalyst
  vtkIOXML
  vtkIONetCDF
  vtkhdf5
)
if(NOT PARAVIEW_USE_MPI)
  message(SEND_ERROR "ParaView must be built with MPI enabled")
endif()

include("${PARAVIEW_USE_FILE}")
include(vtkModuleMacros)
include(vtkMPI)

find_package(Boost 1.53 REQUIRED COMPONENTS
  program_options
  filesystem
  system
  regex
)
include_directories(${Boost_INCLUDE_DIRS})

find_package(HDF5 1.8 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})


#add_subdirectory("source")
set(Executable_SRCS
  source/adaptor.cxx
  source/application.cxx
  source/help.cxx
  source/inporter.cxx
  source/inshimtu.cxx
  source/logger.cxx
  source/notification.cxx
  source/options.cxx
)

add_executable(Inshimtu ${Executable_SRCS})

set_target_properties(Inshimtu PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
)


target_link_libraries(Inshimtu LINK_PRIVATE
  vtkPVPythonCatalyst
  vtkIOXML
  vtkIONetCDF
  vtkNetCDF
  ${Boost_LIBRARIES}
  ${HDF5_LIBRARIES}
)
vtk_mpi_link(Inshimtu)


option(BUILD_TESTING "Build Testing" OFF)
# Setup testing.
if (BUILD_TESTING)
  include(CTest)
  add_test(VTKDataTest Inshimtu ${CMAKE_CURRENT_SOURCE_DIR}/testing/vtkdataTest.py)
  add_test(NetCDFTest Inshimtu ${CMAKE_CURRENT_SOURCE_DIR}/testing/netcdfTest.py)
  add_test(HDF5Test Inshimtu ${CMAKE_CURRENT_SOURCE_DIR}/testing/hdf5Test.py)
endif()
