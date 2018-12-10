cmake_minimum_required(VERSION 3.5.0)
project(Inshimtu)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# NOTE: These settings fix link errors on Cray environment
if (${CMAKE_SYSTEM_NAME} STREQUAL "CrayLinuxEnvironment")
  set(CMAKE_EXE_LINKER_FLAGS "-dynamic -Wl,--no-as-needed -ldl")
endif()


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
  #log
  #log_setup
  thread
  date_time
  filesystem
  system
  regex
)
include_directories(${Boost_INCLUDE_DIRS})


find_package(HDF5 1.8 REQUIRED)
include_directories(${HDF5_INCLUDE_DIRS})


# Allinea profiler
find_library(Allinea_map_sampler_LIBRARY NAMES map-sampler PATHS ${CMAKE_BINARY_DIR} NO_DEFAULT_PATH)
find_library(Allinea_map_sampler_pmpi_LIBRARY NAMES map-sampler-pmpi PATHS ${CMAKE_BINARY_DIR} NO_DEFAULT_PATH)
if (Allinea_map_sampler_LIBRARY AND Allinea_map_sampler_pmpi_LIBRARY)
  set(Allinea_LIBRARIES
    ${Allinea_map_sampler_LIBRARY}
    ${Allinea_map_sampler_pmpi_LIBRARY}
  )

  set(PROFILER_EXE_LINKER_FLAGS "-L${CMAKE_BINARY_DIR} -lmap-sampler-pmpi -lmap-sampler -Wl,--eh-frame-hdr -Wl,-rpath=${CMAKE_BINARY_DIR}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PROFILER_EXE_LINKER_FLAGS}")


  message("Found Allinea_LIBRARIES: ${Allinea_LIBRARIES}")
endif()


#add_subdirectory("source")
set(Executable_SRCS
  source/adaptor.cxx
  source/application.cxx
  source/coordinator.cxx
  source/help.cxx
  source/inporter.cxx
  source/inshimtu.cxx
  source/logger.cxx
  source/notification.cxx
  source/options.cxx
  source/pipeline.cxx
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
# TODO: Fix configuration error (found library, but not CPP version):
#    Found HDF5: HDF5_hdf5_cpp_LIBRARY-NOTFOUND; (found suitable version "1.8.16")
#  ${HDF5_LIBRARIES}
  ${Allinea_LIBRARIES}
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
