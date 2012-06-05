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
#include <vtkRenderer.h>

// ITK
#include "itkImage.h"
#include "itkVectorImage.h"

// Qt
#include <QMainWindow>

// Submodules
#include "ITKVTKCamera/ITKVTKCamera.h"
#include "Layer/Layer.h"

// Custom
#include "PointSelectionStyle2D.h"

class NNFieldInspector : public QMainWindow, public Ui::NNFieldInspector
{
  Q_OBJECT
public:

  typedef itk::Image<itk::CovariantVector<unsigned char, 3>, 2> ImageType;
  typedef itk::VectorImage<float, 2> NNFieldImageType;

  /** Constructor */
  NNFieldInspector();
  NNFieldInspector(const std::string& imageFileName, const std::string& nnFieldFileName);

  /** Set the radius of the patches.*/
  void SetPatchRadius(const unsigned int patchRadius);

public slots:

  void on_actionOpenImage_activated();
  void on_actionOpenNNField_activated();

  void on_actionHelp_activated();
  void on_actionQuit_activated();

  void on_actionFlipHorizontally_activated();
  void on_actionFlipVertically_activated();

  void on_radRGB_clicked();
  void on_radNNField_clicked();

private:

  /** React to a pick event.*/
  void PixelClickedEventHandler(vtkObject* caller, long unsigned int eventId,
                                void* callData);

  /** Functionality shared by all constructors.*/
  void SharedConstructor();

  /** The nearest neighbor field.*/
  NNFieldImageType::Pointer NNField;

  /** The image over which the nearest neighbor field is defined.*/
  ImageType::Pointer Image;

  /** Load an image.*/
  void LoadImage(const std::string& fileName);

  /** Load a nearest neighbor field.*/
  void LoadNNField(const std::string& fileName);

  /** The layer used to display the RGB image.*/
  Layer ImageLayer;

  /** The layer used to display the nearest neighbor field.*/
  Layer NNFieldLayer;

  /** The layer used to do the picking. This layer is always on top and is transparent everywhere
    * except the outline of the current patch and its best match.
    */
  Layer PickLayer;

  /** An object to handle flipping the camera.*/
  ITKVTKCamera Camera;

  /** The radius of the patches.*/
  unsigned int PatchRadius;

  /** The object to handle the picking.*/
  PointSelectionStyle2D* SelectionStyle;

  /** The renderer.*/
  vtkSmartPointer<vtkRenderer> Renderer;

  void UpdateDisplayedImages();

};

#endif
