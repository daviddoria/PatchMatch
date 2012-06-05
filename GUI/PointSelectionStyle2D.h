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

#ifndef PointSelectionStyle2D_H
#define PointSelectionStyle2D_H

// VTK
#include <vtkInteractorStyleImage.h>
#include <vtkCommand.h>

// STL
#include <vector>

// Custom
#include "Coord.h"

// Define interaction style
class PointSelectionStyle2D : public vtkInteractorStyleImage
{
public:
  const static unsigned int PixelClickedEvent = vtkCommand::UserEvent + 1;

  static PointSelectionStyle2D* New();
  vtkTypeMacro(PointSelectionStyle2D, vtkInteractorStyleImage);

  void Initialize() {}

  void SetCurrentRenderer(vtkRenderer*);

private:

  void OnLeftButtonDown();

};

#endif
