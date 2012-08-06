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

template <typename TImage>
class InitializerKnownRegion : public InitializerImage<TImage>
{
public:

  InitializerKnownRegion() {}

  InitializerKnownRegion(TImage* const image, const unsigned int patchRadius) :
    InitializerImage<TImage>(image, patchRadius) {}

  virtual void Initialize(itk::Image<Match, 2>* const initialization)
  {
    // Create a zero region
    itk::Index<2> zeroIndex = {{0,0}};
    itk::Size<2> zeroSize = {{0,0}};
    itk::ImageRegion<2> zeroRegion(zeroIndex, zeroSize);

    // Create an invalid match
    Match invalidMatch;
    invalidMatch.Region = zeroRegion;
    //invalidMatch.Score = std::numeric_limits<float>::max();
    invalidMatch.Score = Match::InvalidScore;

    // Initialize the entire NNfield to be invalid matches
    ITKHelpers::SetImageToConstant(initialization, invalidMatch);

    // Get all of the regions that are entirely inside the image
    itk::ImageRegion<2> internalRegion =
              ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);

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

//     { // Debug only
//     CoordinateImageType::Pointer initialOutput = CoordinateImageType::New();
//     GetPatchCentersImage(this->Output, initialOutput);
//     ITKHelpers::WriteImage(initialOutput.GetPointer(), "InitKnownRegion_Output.mha");
//     }
  }
};

#endif

