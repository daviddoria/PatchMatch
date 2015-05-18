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

#ifndef Match_H
#define Match_H

// ITK
#include "itkImageRegion.h"

// STL
#include <limits>

/** A simple container to pair a region with its patch difference value/score.
 *  It is convenient to store the value along with the location of the match
 *  so that we don't ever have to recompute it.
 */
class Match
{
public:

  Match() : Score(0)
  {
    itk::Index<2> index = {{0,0}};
    itk::Size<2> size = {{0,0}};
    this->Region.SetIndex(index);
    this->Region.SetSize(size);
  }

  void SetRegion(const itk::ImageRegion<2>& region)
  {
    this->Region = region;
  }

  itk::ImageRegion<2> GetRegion() const
  {
    return this->Region;
  }

  void SetScore(const float& score)
  {
    this->Score = score;
  }

  float GetScore() const
  {
    return this->Score;
  }

  bool operator==(const Match &other) const
  {
    if((this->Region != other.GetRegion()) || (this->Score != other.GetScore()))
    {
      return false;
    }

    return true;
  }

private:
  /** The region/patch that describes the 'source' of the match. */
  itk::ImageRegion<2> Region;

  /** The score according to which ever PatchDistanceFunctor is being used. */
  float Score;
};

#endif
