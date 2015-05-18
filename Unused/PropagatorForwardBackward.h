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

// Boost
#include <boost/signals2/signal.hpp>

/** This class propagates good matches. The propagation alternates between forward and backwards propagation
    on successive passes.*/
template <typename TPatchDistanceFunctor>
class PropagatorForwardBackward
{
public:

  /** Propagate good matches from specified offsets. Returns the number of pixels
    * that were propagated to. */
  unsigned int Propagate(NNFieldType* const nnField)
  {
    assert(nnField);
    assert(this->PatchRadius > 0);
    assert(this->PatchDistanceFunctor);

    unsigned int numberOfPropagatedPixels = 0;
    if(this->Forward)
    {
      std::cout << "Propagating forward." << std::endl;

      Propagator<TPatchDistanceFunctor> propagator;
      propagator.SetPatchRadius(this->PatchRadius);
      propagator.SetPatchDistanceFunctor(this->PatchDistanceFunctor);
      propagator.SetForward(true);
      numberOfPropagatedPixels = propagator.Propagate(nnField, force);
    }
    else
    {
      std::cout << "Propagating backward." << std::endl;

      Propagator<TPatchDistanceFunctor> propagator;
      propagator.SetPatchRadius(this->PatchRadius);
      propagator.SetPatchDistanceFunctor(this->PatchDistanceFunctor);
      propagator.SetForward(false);
      numberOfPropagatedPixels = propagator.Propagate(nnField, force);
    }

    this->Forward = !this->Forward;

    return numberOfPropagatedPixels;
  }

  boost::signals2::signal<void (const itk::Index<2>& queryCenter, const itk::Index<2>& matchCenter, const float)> AcceptedSignal;
  boost::signals2::signal<void (NNFieldType*)> PropagatedSignal;
  boost::signals2::signal<void (const itk::Index<2>&)> ProcessPixelSignal;

protected:



};

#endif
