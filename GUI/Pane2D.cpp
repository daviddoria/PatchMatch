#include "Pane2D.h"

// VTK
#include <vtkImageData.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <QVTKWidget.h>

#include "PointSelectionStyle2D.h"

Pane2D::Pane2D(QVTKWidget* inputQVTKWidget)
{
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->SelectionStyle = PointSelectionStyle2D::New();

  this->qvtkWidget = inputQVTKWidget;
  this->qvtkWidget->GetRenderWindow()->AddRenderer(this->Renderer);
  
  this->ImageData = vtkSmartPointer<vtkImageData>::New();

  this->ImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
  
  this->ImageSlice = vtkSmartPointer<vtkImageSlice>::New();
  
  this->CameraLeftToRightVector.resize(3);
  this->CameraLeftToRightVector[0] = -1;
  this->CameraLeftToRightVector[1] = 0;
  this->CameraLeftToRightVector[2] = 0;

  this->CameraBottomToTopVector.resize(3);
  this->CameraBottomToTopVector[0] = 0;
  this->CameraBottomToTopVector[1] = 1;
  this->CameraBottomToTopVector[2] = 0;
}

void Pane2D::SetCameraPosition()
{
  double leftToRight[3] = {this->CameraLeftToRightVector[0], this->CameraLeftToRightVector[1],
  this->CameraLeftToRightVector[2]};
  double bottomToTop[3] = {this->CameraBottomToTopVector[0], this->CameraBottomToTopVector[1],
  this->CameraBottomToTopVector[2]};

  static_cast<PointSelectionStyle2D*>(this->SelectionStyle)->SetImageOrientation(leftToRight, bottomToTop);
  static_cast<PointSelectionStyle2D*>(this->SelectionStyle)->GetCurrentRenderer()->ResetCamera();
  static_cast<PointSelectionStyle2D*>(this->SelectionStyle)->GetCurrentRenderer()->ResetCameraClippingRange();

  this->Renderer->ResetCamera();
  this->Renderer->ResetCameraClippingRange();
  this->Renderer->GetRenderWindow()->Render();
}

void Pane2D::FlipVertically()
{
  this->CameraBottomToTopVector[1] *= -1;
  SetCameraPosition();
}

void Pane2D::FlipHorizontally()
{
  this->CameraLeftToRightVector[0] *= -1;
  SetCameraPosition();
}

void Pane2D::Refresh()
{
  this->qvtkWidget->GetRenderWindow()->Render();
}
