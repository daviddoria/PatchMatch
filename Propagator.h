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

/** A class that traverses a target region and propagates good matches. */
class Propagator
{
public:
  Propagator();

  typedef itk::Image<Match, 2> NNFieldType;

  /** Propagate good matches from specified offsets. */
  template <typename TPatchDistanceFunctor, typename TNeighborFunctor,
          typename TProcessFunctor, typename TAcceptanceTest>
  void Propagate(NNFieldType* const nnField, TPatchDistanceFunctor* patchDistanceFunctor,
                 TNeighborFunctor* neighborFunctor,
                 TProcessFunctor* processFunctor, TAcceptanceTest* acceptanceTest);

  void SetPatchRadius(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
  }
  
protected:
  unsigned int PatchRadius;
};

#include "Propagator.hpp"

#endif