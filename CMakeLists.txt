cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# C++11 support
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

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

# Submodules
if(NOT TARGET PatchComparison)
  add_subdirectory(PatchComparison)
  include_directories(${PatchComparison_includes})
  set(PatchMatch_includes ${PatchMatch_includes} ${PatchComparison_includes})
  set(PatchMatch_libraries ${PatchMatch_libraries} ${PatchComparison_libraries})
endif()

if(NOT ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(PatchMatch_includes ${PatchComparison_includes}
      ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
  set(PatchMatch_libraries ${PatchMatch_libraries} PARENT_SCOPE)
  message("PatchMatch submodule used from ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

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

add_library(PatchMatchHelpers PatchMatchHelpers.cpp)

GET_DIRECTORY_PROPERTY(AllPatchMatchIncludes INCLUDE_DIRECTORIES)
message("Final PatchMatch includes: ${AllPatchMatchIncludes}")
