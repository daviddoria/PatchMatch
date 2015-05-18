# Allow headers in tests to be included like
# #include "ITKHelpers.h" rather than needing
# #include "ITKHelpers/ITKHelpers.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

ADD_EXECUTABLE(TestPatchMatch TestPatchMatch.cpp)
TARGET_LINK_LIBRARIES(TestPatchMatch Mask)

ADD_EXECUTABLE(TestPatchMatchHelpers TestPatchMatchHelpers.cpp)
TARGET_LINK_LIBRARIES(TestPatchMatchHelpers Mask PatchMatchHelpers)
