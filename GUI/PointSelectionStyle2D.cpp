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

#include "PointSelectionStyle2D.h"

// VTK
#include <vtkAbstractPicker.h>
#include <vtkActor2D.h>
#include <vtkCaptionActor2D.h>
#include <vtkCoordinate.h>
#include <vtkFollower.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProp.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVectorText.h>

// STL
#include <sstream>

// Custom
#include "ITKVTKHelpers/ITKHelpers/Helpers/Helpers.h"

vtkStandardNewMacro(PointSelectionStyle2D);

void PointSelectionStyle2D::OnLeftButtonDown()
{
  //std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
  this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
		      this->Interactor->GetEventPosition()[1],
		      0,  // always zero.
                      this->CurrentRenderer);

  double picked[3];
  this->Interactor->GetPicker()->GetPickPosition(picked);
//   std::cout << "PointSelectionStyle2D: Picked point with coordinate: "
//             << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;

  this->InvokeEvent(this->PixelClickedEvent, picked);

  // Forward events
  vtkInteractorStyleImage::OnLeftButtonDown();
}

void PointSelectionStyle2D::SetCurrentRenderer(vtkRenderer* renderer)
{
  vtkInteractorStyleImage::SetCurrentRenderer(renderer);
}
