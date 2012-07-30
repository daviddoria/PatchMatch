# Allow headers in tests to be included like
# #include "PatchMatch.h" rather than needing
# #include "PatchMatch/PatchMatch.h"
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

ADD_EXECUTABLE(PatchMatchInitDemo PatchMatchInitDemo.cpp)
TARGET_LINK_LIBRARIES(PatchMatchInitDemo Mask)

ADD_EXECUTABLE(GroundTruthNNField GroundTruthNNField.cpp)
TARGET_LINK_LIBRARIES(GroundTruthNNField Mask)
