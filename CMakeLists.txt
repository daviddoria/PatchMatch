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

if(NOT TARGET PatchProjection)
  add_subdirectory(PatchProjection)
endif()

# Patch match
add_library(PatchMatch PatchMatch.cpp)
target_link_libraries(PatchMatch Mask)

ADD_EXECUTABLE(PatchMatchDemo PatchMatchDemo.cpp)
TARGET_LINK_LIBRARIES(PatchMatchDemo PatchMatch)

ADD_EXECUTABLE(PatchMatchInitDemo PatchMatchInitDemo.cpp)
TARGET_LINK_LIBRARIES(PatchMatchInitDemo PatchMatch)

SET(BuildGUI ON CACHE BOOL "Build GUI?")
if(BuildGUI)
 add_subdirectory(GUI)
endif(BuildGUI)
