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

// Submodules
#include <Mask/Mask.h>

template <typename TImage>
struct RandomSearch
{
  RandomSearch();

  typedef itk::Image<Match, 2> NNFieldType;

  template <typename TPatchDistanceFunctor>
  void Search(NNFieldType* const nnField, const std::vector<itk::Index<2> >& pixelsToSearch,
              TPatchDistanceFunctor* const patchDistanceFunctor);

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

private:
  TImage* Image;
  Mask* SourceMask;
  unsigned int PatchRadius;
};

#include "RandomSearch.hpp"

#endif
