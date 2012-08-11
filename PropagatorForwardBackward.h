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
template <typename TNeighborFunctor, typename TProcessFunctor,
          typename TAcceptanceTest>
class PropagatorForwardBackward
{
public:
  PropagatorForwardBackward() : Forward(true){}

  typedef itk::Image<Match, 2> MatchImageType;

  /** Propagate good matches from specified offsets. */
  void Propagate(MatchImageType* const nnField)
  {
    Propagator<TNeighborFunctor, TProcessFunctor, TAcceptanceTest> propagator;
    if(this->Forward)
    {
      ForwardPropagationNeighbors neighborFunctor;
      propagator.SetNeighborFunctor(neighborFunctor);
    }
    else
    {
      BackwardPropagationNeighbors neighborFunctor;
      propagator.SetNeighborFunctor(neighborFunctor);
    }

    propagator.SetProcessFunctor(this->ProcessFunctor);
    propagator.SetAcceptanceTest(this->AcceptanceTest);

    this->Forward = !this->Forward;
  }

  void SetProcessFunctor(TProcessFunctor* processFunctor)
  {
    this->ProcessFunctor = processFunctor;
  }

  void SetAcceptanceTest(TAcceptanceTest* acceptanceTest)
  {
    this->AcceptanceTest = acceptanceTest;
  }

private:
  TProcessFunctor* ProcessFunctor;
  TAcceptanceTest* AcceptanceTest;

  bool Forward;
};

#include "PropagatorForwardBackward.hpp"

#endif
