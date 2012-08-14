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

#ifndef PropagatorInterface_H
#define PropagatorInterface_H

// Custom
#include "Match.h"
#include "Process.h"

/** A class that traverses a target region and propagates good matches. */
template <typename TPatchDistanceFunctor, typename TAcceptanceTest>
class PropagatorInterface
{
public:

  PropagatorInterface() : PatchRadius(0), PatchDistanceFunctor(NULL),
                          ProcessFunctor(NULL), AcceptanceTest(NULL){}

  void SetPatchRadius(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
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

  TAcceptanceTest* GetAcceptanceTest()
  {
    return this->AcceptanceTest;
  }

protected:
  unsigned int PatchRadius;
  TPatchDistanceFunctor* PatchDistanceFunctor;
  Process* ProcessFunctor;
  TAcceptanceTest* AcceptanceTest;
};

#endif
