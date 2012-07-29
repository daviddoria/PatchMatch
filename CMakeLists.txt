cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

# ITK
FIND_PACKAGE(ITK REQUIRED ITKCommon ITKIOImageBase ITKIOPNG ITKIOMeta ITKDistanceMap
ITKImageIntensity ITKImageFeature ITKMathematicalMorphology ITKBinaryMathematicalMorphology)
INCLUDE(${USE_ITK_FILE})

# Submodules
if(NOT TARGET Mask)
  add_subdirectory(Mask)
  include_directories(${Mask_includes})
endif()

if(NOT TARGET PatchComparison)
  add_subdirectory(PatchComparison)
  include_directories(${PatchComparison_includes})
endif()

if(NOT ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  set(PatchMatch_includes ${Mask_includes} ${PatchComparison_includes}
      ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
endif()

SET(PatchMatch_BuildExamples ON CACHE BOOL "Build examples?")
if(PatchMatch_BuildExamples)
 add_subdirectory(Examples)
endif()

SET(PatchMatch_BuildTests ON CACHE BOOL "Build tests?")
if(PatchMatch_BuildTests)
 add_subdirectory(Tests)
endif()
