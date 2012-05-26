cmake_minimum_required(VERSION 2.6)

PROJECT(PatchMatch)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")

FIND_PACKAGE(ITK REQUIRED)
INCLUDE(${USE_ITK_FILE})

ADD_EXECUTABLE(PatchMatchDemo PatchMatchDemo.cpp PatchMatch.cpp)
TARGET_LINK_LIBRARIES(PatchMatchDemo ${ITK_LIBRARIES})

add_subdirectory(Mask)