cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

# C++11 support
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

# ITK
if(NOT ITK_FOUND)
  FIND_PACKAGE(ITK REQUIRED ITKCommon ITKIOImageBase ITKIOPNG ITKIOMeta ITKDistanceMap
                            ITKImageIntensity ITKImageFeature ITKMathematicalMorphology
                            ITKBinaryMathematicalMorphology)
  INCLUDE(${USE_ITK_FILE})
endif()

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

SET(PatchMatch_BuildExamples ON CACHE BOOL "Build examples?")
if(PatchMatch_BuildExamples)
 add_subdirectory(Examples)
endif()

SET(PatchMatch_BuildTests ON CACHE BOOL "Build tests?")
if(PatchMatch_BuildTests)
 add_subdirectory(Tests)
endif()

SET(PatchMatch_BuildDrivers ON CACHE BOOL "Build drivers?")
if(PatchMatch_BuildDrivers)
 add_subdirectory(Drivers)
endif()

GET_DIRECTORY_PROPERTY(AllPatchMatchIncludes INCLUDE_DIRECTORIES)
message("Final PatchMatch includes: ${AllPatchMatchIncludes}")
