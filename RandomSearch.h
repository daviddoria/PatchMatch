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
#include "NNField.h"

// Submodules
#include <Mask/Mask.h>

template <typename TImage, typename TPatchDistanceFunctor>
struct RandomSearch
{
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

  void SetPatchDistanceFunctor(TPatchDistanceFunctor* const patchDistanceFunctor)
  {
    this->PatchDistanceFunctor = patchDistanceFunctor;
  }

  TPatchDistanceFunctor* GetPatchDistanceFunctor() const
  {
    return this->PatchDistanceFunctor;
  }

  boost::signals2::signal<void (const itk::Index<2>& queryCenter, const itk::Index<2>& matchCenter, const float)> AcceptedSignal;

  /** Set if the results are truly randomized. */
  void SetRandom(const bool random)
  {
    this->Random = random;
  }

private:
  TImage* Image = nullptr;
  Mask* SourceMask = nullptr;
  unsigned int PatchRadius = 0;
  TPatchDistanceFunctor* PatchDistanceFunctor = nullptr;

  /** Determine if the result should be randomized. This should only be false for testing purposes. */
  bool Random = true;

  /** Seed the random number generator if we are supposed to. */
  void InitRandom();

  std::vector<itk::Index<2> > GetAllPixelIndices(const itk::ImageRegion<2>& region);
};

#include "RandomSearch.hpp"

#endif
