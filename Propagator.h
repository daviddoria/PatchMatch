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

#ifndef Propagator_H
#define Propagator_H

// Custom
#include "Match.h"
#include "PatchMatchHelpers.h"
#include "NNField.h"

/** A class that traverses a target region and propagates good matches. */
template <typename TPatchDistanceFunctor>
class Propagator
{
public:
  /** Propagate good matches from specified offsets. Returns the number of pixels
    * that were successfully propagated to. */
  unsigned int Propagate(NNFieldType* const nnField);

  void SetForward(const bool forward)
  {
      this->Forward = forward;
  }

  void SetPatchRadius(const unsigned int patchRadius)
  {
      this->PatchRadius = patchRadius;
  }

  void SetPatchDistanceFunctor(TPatchDistanceFunctor* const patchDistanceFunctor)
  {
      this->PatchDistanceFunctor = patchDistanceFunctor;
  }

private:
  /** A flag indicating whether we are in the forward (true) or backward (false) pass case. */
  bool Forward = true;

  /** Return either the top and left pixel offsets or bottom and right pixel offsets depending on the Forward flag. */
  std::vector<itk::Offset<2> > GetPropagationOffsets();

  /** The radius of the patches. */
  unsigned int PatchRadius = 5;

  /** The functor used to compare patches. */
  TPatchDistanceFunctor* PatchDistanceFunctor = nullptr;
};

#include "Propagator.hpp"

#endif
