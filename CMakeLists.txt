#TODO: require VERSION 3.2.?
cmake_minimum_required(VERSION 2.8.11)
project(Inshimtu)

#TODO: Replace hack for CMake v2.8 when CMake 3.2 required
# Initialize CXXFLAGS for
set(CMAKE_CXX_FLAGS                "-Wall -std=c++1y")
set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")

# Compiler-specific C++11 activation.
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if (NOT (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8))
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.8 or greater.")
    endif ()
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
else ()
    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()
#TODO:\\

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


#add_subdirectory("source")
set(Executable_SRCS
  source/adaptor.cxx
  source/inshimtu.cxx
  source/application.cxx
  source/notification.cxx
  source/inporter.cxx
  source/logger.cxx
)

add_executable(Inshimtu ${Executable_SRCS})

set_target_properties(Inshimtu PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
)

target_link_libraries(Inshimtu
  vtkPVPythonCatalyst
  vtkIOXML
  vtkIONetCDF
  vtkNetCDF
  hdf5
  ${Boost_LIBRARIES}
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
