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
#include "PropagatorInterface.h"

// Boost
#include <boost/signals2/signal.hpp>

/** A class that traverses a target region and propagates good matches. */
template <typename TPatchDistanceFunctor,
          typename TAcceptanceTest>
class PropagatorForwardBackward : public PropagatorInterface<TPatchDistanceFunctor, TAcceptanceTest>
{
public:
  PropagatorForwardBackward() : PropagatorInterface<TPatchDistanceFunctor, TAcceptanceTest>(),
                                Forward(true) {}

  /** Propagate good matches from specified offsets. */

  void Propagate(PatchMatchHelpers::NNFieldType* const nnField, const bool force = false)
  {
    assert(nnField);
    assert(this->PatchRadius > 0);
    assert(this->ProcessFunctor);
    assert(this->AcceptanceTest);
    assert(this->PatchDistanceFunctor);
    assert(this->ForwardNeighborFunctor);
    assert(this->BackwardNeighborFunctor);

    if(this->Forward)
    {
      std::cout << "Propagating forward." << std::endl;
      this->ProcessFunctor->SetForward(true);

      Propagator<TPatchDistanceFunctor, TAcceptanceTest> propagator;
      propagator.SetPatchRadius(this->PatchRadius);
      propagator.SetNeighborFunctor(this->ForwardNeighborFunctor);
      propagator.SetAcceptanceTest(this->AcceptanceTest);
      propagator.SetPatchDistanceFunctor(this->PatchDistanceFunctor);
      propagator.SetProcessFunctor(this->ProcessFunctor);

      propagator.AcceptedSignal.connect(this->AcceptedSignal);
      propagator.ProcessPixelSignal.connect(this->ProcessPixelSignal);

      propagator.Propagate(nnField, force);
    }
    else
    {
      std::cout << "Propagating backward." << std::endl;
      this->ProcessFunctor->SetForward(false);

      Propagator<TPatchDistanceFunctor, TAcceptanceTest> propagator;
      propagator.SetPatchRadius(this->PatchRadius);
      propagator.SetNeighborFunctor(this->BackwardNeighborFunctor);
      propagator.SetAcceptanceTest(this->AcceptanceTest);
      propagator.SetPatchDistanceFunctor(this->PatchDistanceFunctor);
      propagator.SetProcessFunctor(this->ProcessFunctor);

      propagator.AcceptedSignal.connect(this->AcceptedSignal);
      propagator.ProcessPixelSignal.connect(this->ProcessPixelSignal);

      propagator.Propagate(nnField, force);
    }

    this->Forward = !this->Forward;
  }

  void SetForwardNeighborFunctor(Neighbors* const forwardNeighborFunctor)
  {
    this->ForwardNeighborFunctor = forwardNeighborFunctor;
  }

  void SetBackwardNeighborFunctor(Neighbors* const backwardNeighborFunctor)
  {
    this->BackwardNeighborFunctor = backwardNeighborFunctor;
  }

  boost::signals2::signal<void (const itk::Index<2>& queryCenter, const itk::Index<2>& matchCenter, const float)> AcceptedSignal;
  boost::signals2::signal<void (PatchMatchHelpers::NNFieldType*)> PropagatedSignal;
  boost::signals2::signal<void (const itk::Index<2>&)> ProcessPixelSignal;

protected:

  bool Forward;

  Neighbors* ForwardNeighborFunctor;
  Neighbors* BackwardNeighborFunctor;
};

#endif
