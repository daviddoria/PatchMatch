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

#ifndef AcceptanceTestSourceRegion_H
#define AcceptanceTestSourceRegion_H

// Custom
#include "AcceptanceTest.h"

class AcceptanceTestSourceRegion : public AcceptanceTest
{
public:
  AcceptanceTestSourceRegion(Mask* const mask)
  {
    this->MaskImage = mask;
  }

  virtual bool IsBetterWithScore(const itk::ImageRegion<2>& queryRegion, const Match& oldMatch,
                        const Match& potentialBetterMatch, float& score)
  {
    assert(this->MaskImage);
    return this->MaskImage->IsValid(potentialBetterMatch.GetRegion());
  }

private:
  Mask* MaskImage;
};
#endif
