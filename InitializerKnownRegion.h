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

/** Set every pixel whose surrounding patch is entirely in the source region
  * to have its nearest neighbor as exactly itself. */
template <typename TImage>
class InitializerKnownRegion : public InitializerImage<TImage>
{
public:

  InitializerKnownRegion() {}

  InitializerKnownRegion(TImage* const image, const unsigned int patchRadius) :
    InitializerImage<TImage>(image, patchRadius) {}

  /** Set every pixel whose surrounding patch is entirely in the source region
    * to have its nearest neighbor as exactly itself. Do not modify other pixles in 'initialization'.*/
  virtual void Initialize(itk::Image<Match, 2>* const initialization)
  {
    assert(initialization);
    assert(this->Image);
    assert(initialization->GetLargestPossibleRegion().GetSize() ==
           this->Image->GetLargestPossibleRegion().GetSize());

    // Create a zero region
    itk::Index<2> zeroIndex = {{0,0}};
    itk::Size<2> zeroSize = {{0,0}};
    itk::ImageRegion<2> zeroRegion(zeroIndex, zeroSize);

    // Create an invalid match
    Match invalidMatch;
    invalidMatch.Region = zeroRegion;
    invalidMatch.Score = Match::InvalidScore;

    // Initialize the entire NNfield to be invalid matches
    ITKHelpers::SetImageToConstant(initialization, invalidMatch);

    // Get all of the regions that are entirely inside the image
    itk::ImageRegion<2> internalRegion =
              ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);
    std::cout << "Internal region of " << this->Image->GetLargestPossibleRegion()
              << " is " << internalRegion << std::endl;
    // Set all of the patches that are entirely inside the source region to exactly
    // themselves as their nearest neighbor
    typedef itk::Image<Match, 2> PMImageType;
    itk::ImageRegionIteratorWithIndex<PMImageType> outputIterator(initialization, internalRegion);

    while(!outputIterator.IsAtEnd())
    {
      // Construct the current region
      itk::Index<2> currentIndex = outputIterator.GetIndex();
      itk::ImageRegion<2> currentRegion =
          ITKHelpers::GetRegionInRadiusAroundPixel(currentIndex, this->PatchRadius);

      if(this->SourceMask->IsValid(currentRegion))
      {
        Match selfMatch;
        selfMatch.Region = currentRegion;
        selfMatch.Score = 0.0f;
        outputIterator.Set(selfMatch);
      }

      ++outputIterator;
    }

  }
};

#endif

