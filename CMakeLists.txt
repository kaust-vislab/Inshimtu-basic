cmake_minimum_required(VERSION 3.27)
project(Inshimtu)

set(CMAKE_CXX_STANDARD 20)
set(Boost_NO_WARN_NEW_VERSIONS 1)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and 
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs 
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF) 


# Fail immediately if not using an out-of-source build
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR
    "**** In-source builds are not supported.  Please create a build directory "
    "separate from the source directory")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "CrayLinuxEnvironment")
  if(NOT "$ENV{CRAYPE_LINK_TYPE}" STREQUAL "dynamic")
    # InshimtuLib python bindings require dynamic lib to import
    message(FATAL_ERROR "**** Require 'CRAYPE_LINK_TYPE=dynamic' for CrayLinuxEnvironment")
    # NOTE: These settings fix link errors on Cray environment
    #       if we accepted static link type, set linker flags to support dynamic-only dependencies
    set(CMAKE_EXE_LINKER_FLAGS "-dynamic -Wl,--no-as-needed -ldl")
  endif()
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

SET(CMAKE_CXX_FLAGS_DEBUG "-g -rdynamic")

#------------------------------------------------------------------------------#
# Common dependencies
#------------------------------------------------------------------------------#
set(_components C CXX)
if(CMAKE_Fortran_COMPILER_LOADED)
  list(APPEND _components Fortran)
endif()

find_package(MPI COMPONENTS ${_components})
if(MPI_FOUND)
  # Workaround for various MPI implementations forcing the link of C++ bindings
  add_definitions(-DOMPI_SKIP_MPICXX -DMPICH_SKIP_MPICXX)

  include_directories(SYSTEM ${MPI_INCLUDE_PATH})
  list(APPEND _components MPI)
endif()

find_package(VTK REQUIRED COMPONENTS
  IOXML
  IONetCDF
  hdf5
  netcdf
  ParallelMPI
  IOParallelXML
  FiltersCore
  IOCatalystConduit
  )
message("**** vtk libraries: " ${VTK_LIBRARIES})

find_package(catalyst 2.0 REQUIRED)

find_package(Python3 REQUIRED Interpreter Development)
message("**** python libraries: " ${Python3_LIBRARIES})
message("**** python dirs: " ${Python3_INCLUDE_DIRS})

#add_compile_definitions(BOOST_LOG_DYN_LINK=1)
find_package(Boost 1.85 REQUIRED COMPONENTS
  program_options
  log
  log_setup
  thread
  date_time
  filesystem
  system
  regex
  container
  python310
)
message("**** boost libraries: " ${Boost_LIBRARIES})
message("**** boost log libraries: " ${Boost_LOG_LIBRARY})
message("**** boost log setup libraries: " ${Boost_LOG_SETUP_LIBRARY})
message("**** boost dirs: " ${Boost_INCLUDE_DIRS})


include_directories(${Boost_INCLUDE_DIRS})
include_directories(${Python3_INCLUDE_DIRS})



# TODO: Debugging Info:
#message("VTK_AVAILABLE_COMPONENTS: ${VTK_AVAILABLE_COMPONENTS}")
get_cmake_property(_variableNames VARIABLES)
list (SORT _variableNames)
#foreach (_variableName ${_variableNames})
#  message(STATUS "${_variableName}=${${_variableName}}")
#endforeach()


include_directories("source")
add_executable(inshimtu
  source/inshimtu.cxx
  source/core/application.cxx
  source/core/options.cxx
  source/core/specifications.cxx
  source/sentinels/notification.cxx
  source/sentinels/coordinator.cxx
  source/processing/pipeline.cxx
  source/processing/python.cxx
  source/processing/adaptorV2.cxx
  source/processing/inporter.cxx
  source/processing/inporters/inporterRawNetCDF.cxx
  source/processing/inporters/inporterXMLImage.cxx
  source/processing/inporters/inporterXMLPImage.cxx
  source/processing/inporters/inporterXMLRectilinear.cxx
  source/utils/help.cxx
  source/utils/logger.cxx
)

target_link_libraries(inshimtu
  PRIVATE
    dl # ${SYSTEM_LIBS}
    MPI::MPI_C
    ${Python3_LIBRARIES}
    ${Boost_LIBRARIES}
    ${VTK_LIBRARIES}
    catalyst::catalyst
)

# option(BUILD_TESTING "Build Testing" ON)
# # Setup testing.
# if(BUILD_TESTING)

#   set(Test_SRCS
#     source/tests/test.cxx
#     $<TARGET_OBJECTS:InshimtuObj>
#   )
#   add_executable(testComponents ${Test_SRCS})
#   set_target_properties(testComponents PROPERTIES
#       CXX_STANDARD 17
#       CXX_STANDARD_REQUIRED YES
#   )
#   target_link_libraries(testComponents
#     PRIVATE
#       MPI::MPI_C
#       ${Boost_LIBRARIES}
#       ${VTK_LIBRARIES}
#       ${PYTHON_LIBRARIES}
#       catalyst::catalyst
#     #  ${InshimtuLib}
#   )

#   vtk_module_autoinit(
#     TARGETS testComponents
#     MODULES ${VTK_LIBRARIES}
#   )

#   enable_testing()
#   add_test(TestInputSpecPaths testComponents "InputSpecPaths" ${CMAKE_CURRENT_SOURCE_DIR}/testing)
#   add_test(TestProcessingSpecReadyFile testComponents "ProcessingSpecReadyFile" ${CMAKE_CURRENT_SOURCE_DIR}/testing)
#   add_test(TestProcessingSpecCommands testComponents "ProcessingSpecCommands" ${CMAKE_CURRENT_SOURCE_DIR}/testing)
#   add_test(TestPythonicSpec testComponents "PythonicSpec" ${CMAKE_CURRENT_SOURCE_DIR}/testing)
#   add_test(TestPythonicSpecFile testComponents "PythonicFile" ${CMAKE_CURRENT_SOURCE_DIR}/testing pythonic_tests/pythonicSpec.py)
#   add_test(TestPythonicSpecFile2 testComponents "PythonicFile" ${CMAKE_CURRENT_SOURCE_DIR}/testing pythonic_tests/mitgcmSpec.py)
#   add_test(ConfigurationConfigFile testComponents "ConfigurationConfigFile" ${CMAKE_CURRENT_SOURCE_DIR}/testing configs/mitgcm_compress.json)

# endif()
