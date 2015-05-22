cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

add_subdirectory(CMakeHelpers)

# Add non-compiled files to the project
add_custom_target(PatchMatchSources SOURCES
Match.h
NNField.h
PatchMatch.h
PatchMatch.hpp
PatchMatchHelpers.h
PatchMatchHelpers.hpp
Propagator.h
Propagator.hpp
RandomSearch.h
RandomSearch.hpp
)

# C++11 support
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++11")

#### Eigen ####
# Tell CMake to also look in the source directory to find some .cmake files
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_MODULE_PATH})

# Set the CMake variable to the value of the environment variable.
# If the build is absolutely fresh, this will be the case if the CMake variable EIGEN3_INCLUDE_DIR is not set.
if(NOT EIGEN3_INCLUDE_DIR)
  set(EIGEN3_INCLUDE_DIR $ENV{EIGEN3_INCLUDE_DIR})
  message("Set EIGEN3_INCLUDE_DIR to ${EIGEN3_INCLUDE_DIR} from environment variable of the same name.")
else()
  message("EIGEN3_INCLUDE_DIR is ${EIGEN3_INCLUDE_DIR}")
endif()

if(NOT EIGEN3_FOUND)
  FIND_PACKAGE(Eigen3 REQUIRED) #requires FindEigen3.cmake to be in the source directory
  include_directories(${EIGEN3_INCLUDE_DIR})
endif()

# ITK
if(NOT ITK_FOUND)
  FIND_PACKAGE(ITK REQUIRED ITKCommon ITKIOImageBase ITKIOPNG ITKIOMeta ITKDistanceMap
                            ITKImageIntensity ITKImageFeature ITKMathematicalMorphology
                            ITKBinaryMathematicalMorphology ITKTestKernel)
  INCLUDE(${ITK_USE_FILE})
endif()

# Boost
set(Boost_NO_BOOST_CMAKE TRUE) # seems to avoid "add_library cannot create imported target" errors

FIND_PACKAGE(Boost 1.51 REQUIRED) # The signals2 library is header only, so we don't need to / can't search for it in find_package

IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ENDIF()

UseSubmodule(PatchComparison PatchMatch)

add_library(PatchMatch PatchMatchHelpers.cpp)
set(PatchMatch_libraries ${PatchMatch_libraries} PatchMatch)

CreateSubmodule(PatchMatch)

SET(PatchMatch_BuildExamples OFF CACHE BOOL "Build examples?")
if(PatchMatch_BuildExamples)
 add_subdirectory(Examples)
endif()

SET(PatchMatch_BuildTests ON CACHE BOOL "Build tests?")
if(PatchMatch_BuildTests)
 add_subdirectory(Tests)
endif()

SET(PatchMatch_BuildDrivers OFF CACHE BOOL "Build drivers?")
if(PatchMatch_BuildDrivers)
 add_subdirectory(Drivers)
endif()
