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

#ifndef RandomSearch_HPP
#define RandomSearch_HPP

#include "RandomSearch.h"

// ITK
#include "itkImageRegion.h"

// STL
#include <cassert>
#include <iostream>

// Submodules
#include <ITKHelpers/ITKHelpers.h>

template <typename TImage, typename TPatchDistanceFunctor>
void RandomSearch<TImage, TPatchDistanceFunctor>::
Search(NNFieldType* const nnField)
{
  assert(nnField);
  assert(this->Image);
  assert(this->PatchRadius > 0);
  assert(this->PatchDistanceFunctor);

  assert(nnField->GetLargestPossibleRegion().GetSize()[0] > 0);
  assert(this->Image->GetLargestPossibleRegion().GetSize()[0] > 0);
  assert(nnField->GetLargestPossibleRegion().GetSize() ==
         this->Image->GetLargestPossibleRegion().GetSize());

  InitializeRandomGenerator();

  itk::ImageRegion<2> fullRegion = nnField->GetLargestPossibleRegion();
  itk::ImageRegion<2> internalRegion = ITKHelpers::GetInternalRegion(fullRegion, this->PatchRadius);

  unsigned int numberOfUpdatedPixels = 0;

  if(this->PixelsToProcess.size() == 0)
  {
    this->PixelsToProcess = PatchMatchHelpers::GetAllPixelIndices(internalRegion);
  }

  unsigned int width = internalRegion.GetSize()[0];
  unsigned int height = internalRegion.GetSize()[1];

  // The maximum (first) search radius, as prescribed in PatchMatch paper section 3.2
  unsigned int initialRadius = std::max(width, height);

  for(size_t pixelId = 0; pixelId < this->PixelsToProcess.size(); ++pixelId)
  {
    std::cout << "Searching for a better match for pixel " << pixelId << std::endl;

    itk::Index<2> queryPixel = this->PixelsToProcess[pixelId];

    itk::ImageRegion<2> queryRegion =
      ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, this->PatchRadius);

    assert(fullRegion.IsInside(queryRegion));

    unsigned int radius = initialRadius;

    // Search an exponentially smaller window each time through the loop
    while(radius > this->PatchRadius) // while there is more than just the current patch to search
    {
      itk::ImageRegion<2> searchRegion = ITKHelpers::GetRegionInRadiusAroundPixel(queryPixel, radius);
      searchRegion.Crop(internalRegion);

      itk::ImageRegion<2> randomValidRegion;
      bool hasPixels = GetRandomValidRegion(searchRegion, randomValidRegion);

      if(!hasPixels)
      {
          break;
      }

      // Compute the patch difference
      float dist = this->PatchDistanceFunctor->Distance(randomValidRegion, queryRegion);

      // Construct a match object
      Match potentialMatch;
      potentialMatch.SetRegion(randomValidRegion);
      potentialMatch.SetScore(dist);

      // Store this match as the best match if it meets the criteria.
      // In this class, the criteria is simply that it is
      // better than the current best patch. In subclasses (i.e. GeneralizedPatchMatch),
      // it must be better than the worst patch currently stored.

      Match currentMatch = nnField->GetPixel(queryPixel);

      if(potentialMatch.GetScore() < currentMatch.GetScore())
      {
        nnField->SetPixel(queryPixel, potentialMatch);
        numberOfUpdatedPixels++;
      }

      radius *= this->RegionReductionRatio;
    } // end decreasing radius loop

  } // end loop over target pixels

//  std::cout << "RandomSearch() updated " << numberOfUpdatedPixels << " pixels." << std::endl;
  //std::cout << "RandomSearch: already exact match " << exactMatchPixels << std::endl;
}

template <typename TImage, typename TPatchDistanceFunctor>
bool RandomSearch<TImage, TPatchDistanceFunctor>::
GetRandomValidRegion(const itk::ImageRegion<2>& region, itk::ImageRegion<2>& randomValidRegion)
{
    std::vector<itk::Index<2> > truePixels = ITKHelpers::GetPixelsWithValueInRegion(this->ValidPatchCentersImage, region, true);

    if(truePixels.size() == 0)
    {
        return false;
    }

    unsigned int randomIndex = Helpers::RandomInt(0, truePixels.size() - 1);

    itk::Index<2> randomPixel = truePixels[randomIndex];

    // This is filled instead of returned since it is passed by reference
    randomValidRegion = ITKHelpers::GetRegionInRadiusAroundPixel(randomPixel, this->PatchRadius);

    return true;
}

template <typename TImage, typename TPatchDistanceFunctor>
void RandomSearch<TImage, TPatchDistanceFunctor>::InitializeRandomGenerator()
{
  if(this->Random)
  {
    srand(time(NULL));
  }
  else
  {
    srand(0);
  }
}

#endif
