cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

# Eigen
include_directories(/media/portable/src/Eigen)

# VTK
FIND_PACKAGE(ITK REQUIRED ITKCommon ITKIOImageBase
ITKImageIntensity ITKImageFeature ITKMathematicalMorphology ITKBinaryMathematicalMorphology
ITKDeprecated)
INCLUDE(${USE_ITK_FILE})

# Submodules
if(NOT TARGET Mask)
  add_subdirectory(Mask)
endif()

if(NOT TARGET PatchComparison)
  add_subdirectory(PatchComparison)
endif()

SET(PatchMatch_BuildGUI ON CACHE BOOL "Build GUI?")
if(PatchMatch_BuildGUI)
 add_subdirectory(GUI)
endif()

SET(PatchMatch_BuildExamples ON CACHE BOOL "Build examples?")
if(PatchMatch_BuildExamples)
 add_subdirectory(Examples)
endif()

SET(PatchMatch_BuildTests ON CACHE BOOL "Build tests?")
if(PatchMatch_BuildTests)
 add_subdirectory(Tests)
endif()
