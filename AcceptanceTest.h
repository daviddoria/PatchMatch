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

#ifndef AcceptanceTest_H
#define AcceptanceTest_H

// ITK
#include "itkIndex.h"

// STL
#include <limits>

// Submodules
#include <Helpers/Helpers.h>

// Custom
#include "Match.h"

/** A class that tests if a 'match' is better than the current match at 'index'. */
class AcceptanceTest
{
public:
  AcceptanceTest() : IncludeInScore(true) {}

  /** This function is used to determine if the acceptance test and return how well it did in 'score'. */
  virtual bool IsBetterWithScore(const itk::ImageRegion<2>& queryRegion, const Match& oldMatch,
                                 const Match& potentialBetterMatch, float& score) = 0;

  /** This function is used to determine if the acceptance test passed without caring what the score was. */
  virtual bool IsBetter(const itk::ImageRegion<2>& queryRegion, const Match& oldMatch,
                        const Match& potentialBetterMatch)
  {
    float score; // unused
    return IsBetterWithScore(queryRegion, oldMatch, potentialBetterMatch, score);
  }

  /** Set if this acceptance test will contribute to a composite acceptance test score. */
  void SetIncludeInScore(const bool includeInScore)
  {
    this->IncludeInScore = includeInScore;
  }

  /** Determine if this acceptance test will contribute to a composite acceptance test score. */
  bool GetIncludeInScore()
  {
    return IncludeInScore;
  }

protected:
  /** Determine if this acceptance test will contribute to a composite acceptance test score. */
  bool IncludeInScore;
};

/** A class that tests if a 'match' is better than the current match at 'index'. */
template <typename TImage>
class AcceptanceTestImage : public AcceptanceTest
{
public:
  AcceptanceTestImage() : AcceptanceTest(), Image(NULL), PatchRadius(0){}

  void SetPatchRadius(const unsigned int patchRadius)
  {
    this->PatchRadius = patchRadius;
  }

  virtual bool IsBetterWithScore(const itk::ImageRegion<2>& queryRegion, const Match& oldMatch,
                        const Match& potentialBetterMatch, float& score) = 0;

  void SetImage(TImage* const image)
  {
    this->Image = image;
  }

protected:
  TImage* Image;

  unsigned int PatchRadius;
};

#endif
