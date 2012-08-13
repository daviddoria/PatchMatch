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
#include "Process.h"

/** A class that traverses a target region and propagates good matches. */
template <typename TPatchDistanceFunctor, typename TNeighborFunctor,
          typename TAcceptanceTest>
class Propagator
{
public:
  Propagator();

  typedef itk::Image<Match, 2> NNFieldType;

  /** Propagate good matches from specified offsets. */
  void Propagate(NNFieldType* const nnField);

  void SetPatchRadius(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
  }

  void SetNeighborFunctor(TNeighborFunctor* neigborFunctor)
  {
    this->NeighborFunctor = neigborFunctor;
  }

  void SetProcessFunctor(Process* processFunctor)
  {
    this->ProcessFunctor = processFunctor;
  }

  void SetAcceptanceTest(TAcceptanceTest* acceptanceTest)
  {
    this->AcceptanceTest = acceptanceTest;
  }

  void SetPatchDistanceFunctor(TPatchDistanceFunctor* patchDistanceFunctor)
  {
    this->PatchDistanceFunctor = patchDistanceFunctor;
  }

protected:
  unsigned int PatchRadius;

  TPatchDistanceFunctor* PatchDistanceFunctor;
  TNeighborFunctor* NeighborFunctor;
  Process* ProcessFunctor;
  TAcceptanceTest* AcceptanceTest;
};

#include "Propagator.hpp"

#endif
