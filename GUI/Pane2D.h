#ifndef PANE2D_H
#define PANE2D_H

// ITK
#include "itkImage.h"

// VTK
#include <vtkSmartPointer.h>
class vtkImageData;
class vtkImageSliceMapper;
class vtkImageSlice;
class vtkRenderer;
class QVTKWidget;

// Custom
class PointSelectionStyle2D;

struct Pane2D
{
  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;
  
  Pane2D(QVTKWidget* qvtkWidget);
  
  ImageType::Pointer Image;
  vtkSmartPointer<vtkImageData> ImageData;
  
  vtkSmartPointer<vtkImageSliceMapper> ImageSliceMapper;
  vtkSmartPointer<vtkImageSlice> ImageSlice;

  void FlipVertically();
  void FlipHorizontally();

  PointSelectionStyle2D* SelectionStyle;

  vtkSmartPointer<vtkRenderer> Renderer;

  QVTKWidget* qvtkWidget;

private:
  std::vector<float> CameraLeftToRightVector;
  std::vector<float> CameraBottomToTopVector;
  void SetCameraPosition();

  void Refresh();
};

#endif
