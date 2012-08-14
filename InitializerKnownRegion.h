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

#ifndef InitializerKnownRegion_H
#define InitializerKnownRegion_H

#include "Initializer.h"

// Custom
#include "Match.h"
#include "PatchMatchHelpers.h"

/** Set every pixel whose surrounding patch is entirely in the source region
  * to have its nearest neighbor as exactly itself. */
class InitializerKnownRegion : public InitializerPatch
{
public:

  InitializerKnownRegion() {}

  InitializerKnownRegion(const unsigned int patchRadius) :
    InitializerPatch(patchRadius) {}

  /** Set every pixel whose surrounding patch is entirely in the source region
    * to have its nearest neighbor as exactly itself. These pixels are also marked as verified automatically.
    * Do not modify other pixels in 'initialization'.*/
  virtual void Initialize(PatchMatchHelpers::NNFieldType* const initialization)
  {
    assert(initialization);

    assert(initialization->GetLargestPossibleRegion().GetSize()[0] > 0);

    itk::ImageRegion<2> region = initialization->GetLargestPossibleRegion();

    // Create a zero region
    itk::Index<2> zeroIndex = {{0,0}};
    itk::Size<2> zeroSize = {{0,0}};
    itk::ImageRegion<2> zeroRegion(zeroIndex, zeroSize);

    // Get all of the regions that are entirely inside the image
    itk::ImageRegion<2> internalRegion =
              ITKHelpers::GetInternalRegion(region, this->PatchRadius);
//     std::cout << "Internal region of " << region
//               << " is " << internalRegion << std::endl;
    // Set all of the patches that are entirely inside the source region to exactly
    // themselves as their nearest neighbor

    itk::ImageRegionIteratorWithIndex<PatchMatchHelpers::NNFieldType> outputIterator(initialization, internalRegion);

    while(!outputIterator.IsAtEnd())
    {
      // Construct the current region
      itk::Index<2> currentIndex = outputIterator.GetIndex();
      itk::ImageRegion<2> currentRegion =
          ITKHelpers::GetRegionInRadiusAroundPixel(currentIndex, this->PatchRadius);

      if(this->SourceMask->IsValid(currentRegion))
      {
        Match selfMatch;
        selfMatch.SetRegion(currentRegion);
        selfMatch.SetSSDScore(0.0f);
        selfMatch.SetVerificationScore(0.0f);
        selfMatch.SetVerified(true);

        MatchSet currentMatches = outputIterator.Get();
        currentMatches.AddMatch(selfMatch);
        outputIterator.Set(currentMatches);
      }

      ++outputIterator;
    }

  }

};

#endif

