/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef NNFieldInspector_H
#define NNFieldInspector_H

#include "ui_NNFieldInspector.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkSeedWidget.h>
#include <vtkPointHandleRepresentation2D.h>

// ITK
#include "itkImage.h"
#include "itkVectorImage.h"

// Qt
#include <QMainWindow>

// Submodules
#include "ITKVTKCamera/ITKVTKCamera.h"

// Custom
#include "Pane2D.h"
#include "PointSelectionStyle2D.h"

// Forward declarations
class vtkActor;
class vtkImageData;
class vtkImageActor;
class vtkPolyData;
class vtkPolyDataMapper;
class vtkRenderer;

class NNFieldInspector : public QMainWindow, public Ui::NNFieldInspector
{
  Q_OBJECT
public:

  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;
  typedef itk::VectorImage<float, 2> NNFieldImageType;

  // Constructor
  NNFieldInspector();
  NNFieldInspector(const std::string& imageFileName, const std::string& nnFieldFileName);

public slots:
  void on_actionOpenImageLeft_activated();
  void on_actionOpenImageRight_activated();

  void on_actionHelp_activated();
  void on_actionQuit_activated();
  
  void on_actionFlipLeftHorizontally_activated();
  void on_actionFlipLeftVertically_activated();
  void on_actionFlipRightHorizontally_activated();
  void on_actionFlipRightVertically_activated();

private:

  void PixelClickedEventHandler(vtkObject* caller, long unsigned int eventId,
                                void* callData);

  void SharedConstructor();

  NNFieldImageType::Pointer NNField;
  ImageType::Pointer Image;

  /** Load an image*/
  void LoadImage(Pane2D* const pane, const std::string& fileName);

  Pane2D* LeftPane;
  Pane2D* RightPane;

  ITKVTKCamera Camera;
};

#endif
