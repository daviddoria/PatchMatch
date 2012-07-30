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

#ifndef PatchMatchRings_HPP
#define PatchMatchRings_HPP

#include "PatchMatchRings.h"

template <typename TImage>
PatchMatchRings<TImage>::PatchMatchRings() : PatchMatch<TImage>()
{
}

template <typename TImage>
bool PatchMatchRings<TImage>::AllowPropagationFrom(const itk::Index<2>& potentialPropagationPixel)
{
  if(this->SourceMask->IsValid(potentialPropagationPixel))
  {
    return true;
  }

  return false;
}

#endif
