cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

# Eigen
include_directories(/media/portable/src/Eigen)

# VTK
FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${USE_ITK_FILE})

# Necessary if ITK is built with ITKVtkGlue (should not be required, I'd consider this a bug).
FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${USE_VTK_FILE})

# Submodules
add_subdirectory(Mask)
add_subdirectory(PatchProjection)

# Patch match
add_library(PatchMatch PatchMatch.cpp)
target_link_libraries(PatchMatch Mask)

ADD_EXECUTABLE(PatchMatchDemo PatchMatchDemo.cpp)
TARGET_LINK_LIBRARIES(PatchMatchDemo PatchMatch)

ADD_EXECUTABLE(PatchMatchInitDemo PatchMatchInitDemo.cpp)
TARGET_LINK_LIBRARIES(PatchMatchInitDemo PatchMatch)

# Bidirectional similarity inpainting
add_library(BDSInpainting BDSInpainting.cpp)
target_link_libraries(BDSInpainting PatchMatch)

ADD_EXECUTABLE(BDSInpaintingDemo BDSInpaintingDemo.cpp)
TARGET_LINK_LIBRARIES(BDSInpaintingDemo BDSInpainting)

SET(BuildGUI ON CACHE BOOL "Build GUI?")
if(BuildGUI)
 add_subdirectory(GUI)
endif(BuildGUI)
