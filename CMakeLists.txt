cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${USE_ITK_FILE})

add_subdirectory(Mask)

add_library(PatchMatch PatchMatch.cpp)
target_link_libraries(PatchMatch Mask)

ADD_EXECUTABLE(PatchMatchDemo PatchMatchDemo.cpp PatchMatch.cpp)
TARGET_LINK_LIBRARIES(PatchMatchDemo PatchMatch)

ADD_EXECUTABLE(PatchMatchInitDemo PatchMatchInitDemo.cpp)
TARGET_LINK_LIBRARIES(PatchMatchInitDemo PatchMatch)

add_library(BDSInpainting PatchMatch.cpp BDSInpainting.cpp)
target_link_libraries(BDSInpainting PatchMatch)

ADD_EXECUTABLE(BDSInpaintingDemo BDSInpaintingDemo.cpp)
TARGET_LINK_LIBRARIES(BDSInpaintingDemo BDSInpainting)

SET(BuildGUI ON CACHE BOOL "Build GUI?" FORCE)

# if(BuildGUI EQUALS ON)
#  add_subdirectory(GUI)
# endif(BuildGUI EQUALS ON)

add_subdirectory(GUI)
