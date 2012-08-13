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

#ifndef PropagatorForwardBackward_H
#define PropagatorForwardBackward_H

// Custom
#include "Propagator.h"

/** A class that traverses a target region and propagates good matches. */
template <typename TPatchDistanceFunctor, typename TProcessFunctor,
          typename TAcceptanceTest>
class PropagatorForwardBackward
{
public:
  PropagatorForwardBackward() : PatchRadius(0), Forward(true), PatchDistanceFunctor(NULL),
                                ProcessFunctor(NULL), AcceptanceTest(NULL){}

  /** Propagate good matches from specified offsets. */

  void Propagate(PatchMatchHelpers::NNFieldType* const nnField)
  {
    assert(nnField);
    assert(this->PatchRadius > 0);
    assert(this->ProcessFunctor);
    assert(this->AcceptanceTest);
    assert(this->PatchDistanceFunctor);

    if(this->Forward)
    {
      std::cout << "Propagating forward." << std::endl;
      this->ProcessFunctor->SetForward(true);

      ForwardPropagationNeighbors neighborFunctor;
      Propagator<TPatchDistanceFunctor, ForwardPropagationNeighbors,
                 TAcceptanceTest> propagator;
      propagator.SetPatchRadius(this->PatchRadius);
      propagator.SetNeighborFunctor(&neighborFunctor);
      propagator.SetAcceptanceTest(this->AcceptanceTest);
      propagator.SetPatchDistanceFunctor(this->PatchDistanceFunctor);
      propagator.SetProcessFunctor(this->ProcessFunctor);
      propagator.Propagate(nnField);
    }
    else
    {
      std::cout << "Propagating backward." << std::endl;
      this->ProcessFunctor->SetForward(false);
      
      BackwardPropagationNeighbors neighborFunctor;
      Propagator<TPatchDistanceFunctor, BackwardPropagationNeighbors,
                 TAcceptanceTest> propagator;
      propagator.SetPatchRadius(this->PatchRadius);
      propagator.SetNeighborFunctor(&neighborFunctor);
      propagator.SetAcceptanceTest(this->AcceptanceTest);
      propagator.SetPatchDistanceFunctor(this->PatchDistanceFunctor);
      propagator.SetProcessFunctor(this->ProcessFunctor);
      propagator.Propagate(nnField);
    }

    this->Forward = !this->Forward;
  }

  void SetPatchRadius(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
  }

  void SetProcessFunctor(TProcessFunctor* processFunctor)
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

  bool Forward;

  TPatchDistanceFunctor* PatchDistanceFunctor;
  TProcessFunctor* ProcessFunctor;
  TAcceptanceTest* AcceptanceTest;
};

#endif
