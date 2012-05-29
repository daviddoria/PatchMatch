
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

FIND_PACKAGE(Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${USE_VTK_FILE})

QT4_WRAP_UI(UISrcs NNFieldInspector.ui)
QT4_WRAP_CPP(MOCSrcs NNFieldInspector.h)

add_subdirectory(ITKVTKHelpers)

add_subdirectory(ITKVTKCamera)
add_subdirectory(VTKHelpers)

add_executable(NNFieldInspector NNFieldInspectorDriver.cpp
NNFieldInspector.cpp
Pane2D.cpp PointSelectionStyle2D.cpp
${UISrcs} ${MOCSrcs})

TARGET_LINK_LIBRARIES(NNFieldInspector ${VTK_LIBRARIES} ${ITK_LIBRARIES}
VTKHelpers ITKVTKCamera ITKVTKHelpers ITKHelpers)
