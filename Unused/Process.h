/*=========================================================================
 *
 *  Copyright David Doria 2012 daviddoria@gmail.com
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

#ifndef Process_H
#define Process_H

// STL
#include <functional>
#include <vector>

// Custom
#include "Match.h"
#include "PatchMatchHelpers.h"

// Submodules
#include <Mask/Mask.h>

/** This class decides which pixels are allowed to be propagated to (aka be processed/receive new information).
 * (The reverse of PropagationAllowed, which determines if pixels can be propagated.)*/
struct Process
{
  Process(Mask* const mask, PatchMatchHelpers::NNFieldType* const nnField) : NNField(nnField), MaskImage(mask)
  {}

  Process(Mask* const mask) : NNField(nullptr), MaskImage(mask), Forward(true)
  {}

  void SetNNField(PatchMatchHelpers::NNFieldType* const nnField)
  {
    this->NNField = nnField;
  }

  void SetMask(Mask* const mask)
  {
    this->MaskImage = mask;
  }

  void SetForward(const bool forward)
  {
    this->Forward = forward;
  }

protected:
  PatchMatchHelpers::NNFieldType* NNField = nullptr;
  Mask* MaskImage = nullptr;
  bool Forward = true;
};


#endif
