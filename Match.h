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

#ifndef MATCH_H
#define MATCH_H

// ITK
#include "itkImageRegion.h"

// STL
#include <limits>

// Submodules
#include <Helpers/Helpers.h>

/** A simple container to pair a region with its patch difference value/score. */
struct Match
{
  itk::ImageRegion<2> Region;
  float Score;

  static constexpr float InvalidScore = std::numeric_limits< float >::quiet_NaN();
  bool IsValid()
  {
    if(Helpers::IsNaN(this->Score))
    {
      return false;
    }

    if(this->Region.GetSize()[0] == 0 || this->Region.GetSize()[1] == 0)
    {
      return false;
    }

    return true;
  }
};

#endif
