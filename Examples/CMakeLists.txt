# Allow headers in tests to be included like
# #include "ITKHelpers.h" rather than needing
# #include "ITKHelpers/ITKHelpers.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

# Patch match
ADD_EXECUTABLE(PatchMatchDemo PatchMatchDemo.cpp)
TARGET_LINK_LIBRARIES(PatchMatchDemo Mask)

ADD_EXECUTABLE(PatchMatchInitDemo PatchMatchInitDemo.cpp)
TARGET_LINK_LIBRARIES(PatchMatchInitDemo Mask)

ADD_EXECUTABLE(GroundTruthNNField GroundTruthNNField.cpp)
TARGET_LINK_LIBRARIES(GroundTruthNNField Mask)
