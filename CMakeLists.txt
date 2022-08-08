cmake_minimum_required(VERSION 3.5.0)
project(Inshimtu)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(${CMAKE_SYSTEM_NAME} STREQUAL "CrayLinuxEnvironment")
  if(NOT "$ENV{CRAYPE_LINK_TYPE}" STREQUAL "dynamic")
    # InshimtuLib python bindings require dynamic lib to import
    message(FATAL_ERROR "Require 'CRAYPE_LINK_TYPE=dynamic' for CrayLinuxEnvironment")
    # NOTE: These settings fix link errors on Cray environment
    #       if we accepted static link type, set linker flags to support dynamic-only dependencies
    set(CMAKE_EXE_LINKER_FLAGS "-dynamic -Wl,--no-as-needed -ldl")
  endif()
endif()

#find_package (Python COMPONENTS Interpreter Development REQUIRED)
#set(PYTHONLIBS_VERSION_STRING ${Python_VERSION})
#set(PYTHON_INCLUDE_DIR ${Python_INCLUDE_DIRS})
#set(PYTHON_LIBRARIES ${Python_LIBRARIES})
#include_directories(${Python_INCLUDE_DIRS})
#message(STATUS "Python3 include dirs: ${Python_INCLUDE_DIRS}")

find_package(ParaView 5.9.1 COMPONENTS
  PythonCatalyst
)

if(ParaView_FOUND)
  find_package(VTK REQUIRED COMPONENTS
    IOXML
    IONetCDF
    hdf5
    netcdf
    ParallelMPI
  #  NO_MODULE
  )
endif()

if(NOT PARAVIEW_USE_MPI) # VTK_ParallelMPI_FOUND
  message(FATAL_ERROR "ParaView must be built with MPI enabled")
endif()

#include(${VTK_USE_FILE})

include_directories(SYSTEM "${ParaView_PREFIX_PATH}/include")
include_directories(SYSTEM "${ParaView_PREFIX_PATH}/include/paraview-${ParaView_VERSION_MAJOR}.${ParaView_VERSION_MINOR}")

#include(vtkModuleMacros)
#include(MPI)

if(ParaView_VERSION_MINOR GREATER_EQUAL 6)
  set(PV_LIB_SUFFIX "-pv${ParaView_VERSION_MAJOR}.${ParaView_VERSION_MINOR}")
else()
  set(PV_LIB_SUFFIX "")
endif()
message(STATUS "Paraview lib suffix: ${PV_LIB_SUFFIX}")
message(STATUS "Paraview version: ${ParaView_VERSION}")
message(STATUS "Paraview version minor: ${ParaView_VERSION_MINOR}")


add_compile_definitions(BOOST_LOG_DYN_LINK=1)
find_package(Boost 1.67 REQUIRED COMPONENTS
  program_options
  log
  log_setup
  thread
  date_time
  filesystem
  system
  regex
  container
  python38
)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Python3_INCLUDE_DIRS})

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


# TODO: Debugging Info:
#message("VTK_AVAILABLE_COMPONENTS: ${VTK_AVAILABLE_COMPONENTS}")
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#  message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


include_directories("source")
set(Library_SRCS
  source/core/application.cxx
  source/core/options.cxx
  source/core/specifications.cxx
  source/sentinels/notification.cxx
  source/sentinels/coordinator.cxx
  source/processing/pipeline.cxx
  source/processing/python.cxx
  source/processing/adaptor.cxx
  source/processing/inporter.cxx
  source/processing/inporters/inporterRawNetCDF.cxx
  source/processing/inporters/inporterXMLImage.cxx
  source/processing/inporters/inporterXMLPImage.cxx
  source/utils/help.cxx
  source/utils/logger.cxx
)


add_library(InshimtuObj OBJECT
  ${Library_SRCS}
  )
set_target_properties(InshimtuObj PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    POSITION_INDEPENDENT_CODE 1
)


add_library(InshimtuLib SHARED
  $<TARGET_OBJECTS:InshimtuObj>
  )
set_target_properties(InshimtuLib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
  PREFIX ""
)
target_link_libraries(InshimtuLib 
  PRIVATE
    ${Boost_LIBRARIES}
    ${VTK_LIBRARIES}
    ${ParaView_LIBRARIES}
    ${PYTHON_LIBRARIES}
)
if(VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
  vtk_module_autoinit(
    TARGETS InshimtuLib
    MODULES ${VTK_LIBRARIES}
  )
else()
  vtk_mpi_link(InshimtuLib)
endif()


set(Executable_SRCS
  source/inshimtu.cxx
  $<TARGET_OBJECTS:InshimtuObj>
)

add_executable(Inshimtu ${Executable_SRCS})
set_target_properties(Inshimtu PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)
target_link_libraries(Inshimtu
  PRIVATE
    # TODO: Fix linker errors:
    dl # ${SYSTEM_LIBS}
    ${Boost_LIBRARIES}
    ${VTK_LIBRARIES}
    ${ParaView_LIBRARIES}
    ${PYTHON_LIBRARIES}
    # TODO: Fix configuration error (found library, but not CPP version):
    #    Found HDF5: HDF5_hdf5_cpp_LIBRARY-NOTFOUND; (found suitable version "1.8.16")
    #  ${HDF5_LIBRARIES}
    # TODO: What is the best way to enable Python?
    #       https://vsamy.github.io/en/blog/boost-python-cmake-build
    #       https://stackoverflow.com/questions/35367469/force-cmake-visualstudio-to-use-static-libs-of-boost-python/35440164#35440164
    #       https://www.boost.org/doc/libs/1_60_0/libs/python/doc/html/tutorial/tutorial/embedding.html
    #    ${InshimtuLib}
    ${Allinea_LIBRARIES}
)
if(VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
  vtk_module_autoinit(
    TARGETS Inshimtu
    MODULES ${VTK_LIBRARIES}
  )
else()
  vtk_mpi_link(Inshimtu)
endif()


option(BUILD_TESTING "Build Testing" ON)
# Setup testing.
if(BUILD_TESTING)

  set(Test_SRCS
    source/tests/test.cxx
    $<TARGET_OBJECTS:InshimtuObj>
  )
  add_executable(testComponents ${Test_SRCS})
  set_target_properties(testComponents PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED YES
  )
  target_link_libraries(testComponents
    PRIVATE
      ${Boost_LIBRARIES}
      ${VTK_LIBRARIES}
      ${ParaView_LIBRARIES}
      ${PYTHON_LIBRARIES}
    #  ${InshimtuLib}
  )
  if(VTK_VERSION VERSION_GREATER_EQUAL "8.90.0")
    vtk_module_autoinit(
      TARGETS testComponents
      MODULES ${VTK_LIBRARIES}
    )
  else()
    vtk_mpi_link(testComponents)
  endif()

  #include(CTest) # TODO: Only for Dashboard support
  enable_testing()

  add_test(TestInputSpecPaths testComponents "InputSpecPaths" ${CMAKE_CURRENT_SOURCE_DIR}/testing)
  add_test(TestProcessingSpecReadyFile testComponents "ProcessingSpecReadyFile" ${CMAKE_CURRENT_SOURCE_DIR}/testing)
  add_test(TestProcessingSpecCommands testComponents "ProcessingSpecCommands" ${CMAKE_CURRENT_SOURCE_DIR}/testing)
  add_test(TestPythonicSpec testComponents "PythonicSpec" ${CMAKE_CURRENT_SOURCE_DIR}/testing)
  add_test(TestPythonicSpecFile testComponents "PythonicFile" ${CMAKE_CURRENT_SOURCE_DIR}/testing pythonic_tests/pythonicSpec.py)
  add_test(TestPythonicSpecFile2 testComponents "PythonicFile" ${CMAKE_CURRENT_SOURCE_DIR}/testing pythonic_tests/mitgcmSpec.py)
  add_test(ConfigurationConfigFile testComponents "ConfigurationConfigFile" ${CMAKE_CURRENT_SOURCE_DIR}/testing configs/mitgcm_compress.json)

endif()
