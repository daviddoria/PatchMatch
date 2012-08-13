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

#ifndef RandomSearch_H
#define RandomSearch_H

// ITK
#include "itkImage.h"

// Custom
#include "Match.h"
#include "Process.h"

// Submodules
#include <Mask/Mask.h>

template <typename TImage, typename TPatchDistanceFunctor,
          typename TAcceptanceTest>
struct RandomSearch
{
  RandomSearch();

  typedef itk::Image<Match, 2> NNFieldType;

  void Search(NNFieldType* const nnField);

  void SetPatchRadius(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
  }

  void SetImage(TImage* const image)
  {
    this->Image = image;
  }

  void SetSourceMask(Mask* const mask)
  {
    this->SourceMask = mask;
  }

  void SetProcessFunctor(Process* const processFunctor)
  {
    this->ProcessFunctor = processFunctor;
  }

  void SetPatchDistanceFunctor(TPatchDistanceFunctor* const patchDistanceFunctor)
  {
    this->PatchDistanceFunctor = patchDistanceFunctor;
  }

  void SetAcceptanceTest(TAcceptanceTest* const acceptanceTest)
  {
    this->AcceptanceTest = acceptanceTest;
  }

private:
  TImage* Image;
  Mask* SourceMask;
  unsigned int PatchRadius;
  TPatchDistanceFunctor* PatchDistanceFunctor;
  Process* ProcessFunctor;
  TAcceptanceTest* AcceptanceTest;
};

#include "RandomSearch.hpp"

#endif
