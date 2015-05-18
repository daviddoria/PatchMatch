# Allow headers in tests to be included like
# #include "PatchMatch.h" rather than needing
# #include "PatchMatch/PatchMatch.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

ADD_EXECUTABLE(PatchMatch PatchMatch.cpp)
TARGET_LINK_LIBRARIES(PatchMatch Mask PatchMatchHelpers)
