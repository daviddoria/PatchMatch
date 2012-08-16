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
#include "Process.h"
#include "PropagatorInterface.h"
#include "Neighbors.h"

// Boost
#include <boost/signals2/signal.hpp>

/** A class that traverses a target region and propagates good matches. */
template <typename TPatchDistanceFunctor,
          typename TAcceptanceTest>
class Propagator : public PropagatorInterface<TPatchDistanceFunctor, TAcceptanceTest>
{
public:
  Propagator();

  /** Propagate good matches from specified offsets. */
  void Propagate(PatchMatchHelpers::NNFieldType* const nnField, const bool force = false);

  void SetNeighborFunctor(Neighbors* neigborFunctor)
  {
    this->NeighborFunctor = neigborFunctor;
  }

  boost::signals2::signal<void (PatchMatchHelpers::NNFieldType*)> PropagatedSignal;

  boost::signals2::signal<void (const itk::Index<2>& queryCenter, const itk::Index<2>& matchCenter, const float)> AcceptedSignal;

protected:
  Neighbors* NeighborFunctor;
};

#include "Propagator.hpp"

#endif
