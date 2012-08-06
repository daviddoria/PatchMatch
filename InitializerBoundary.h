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

#ifndef InitializerBoundary_H
#define InitializerBoundary_H

#include "Initializer.h"

template <typename TImage>
class InitializerBoundary : public InitializerImage<TImage>
{
public:

  InitializerBoundary(TImage* const image, const unsigned int patchRadius) :
    InitializerImage<TImage>(image, patchRadius) {}
    
  virtual void Initialize()
  {
    itk::ImageRegion<2> internalRegion =
             ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(), this->PatchRadius);

    // Expand the hole
    Mask::Pointer expandedSourceMask = Mask::New();
    expandedSourceMask->DeepCopyFrom(this->SourceMask);
    expandedSourceMask->ExpandHole(this->PatchRadius);

    // Get the expanded boundary
    Mask::BoundaryImageType::Pointer boundaryImage = Mask::BoundaryImageType::New();
    unsigned char outputBoundaryPixelValue = 255;
    expandedSourceMask->FindBoundary(boundaryImage.GetPointer(), Mask::VALID, outputBoundaryPixelValue);
    ITKHelpers::WriteImage(boundaryImage.GetPointer(), "ExpandedBoundary.png");

    // Get the boundary pixels
    std::vector<itk::Index<2> > boundaryIndices =
          ITKHelpers::GetPixelsWithValue(boundaryImage.GetPointer(), outputBoundaryPixelValue);

    std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();

    for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
    {
      // Construct the current region
      itk::Index<2> targetPixelIndex = targetPixels[targetPixelId];

      // Only do this for pixels which have not already been initialized to themselves
      if(this->Output->GetPixel(targetPixelIndex).Score == 0)
      {
        continue;
      }

      itk::ImageRegion<2> currentRegion =
            ITKHelpers::GetRegionInRadiusAroundPixel(targetPixelIndex, this->PatchRadius);

      if(!this->Image->GetLargestPossibleRegion().IsInside(currentRegion))
      {
        continue;
      }
      // Find the nearest valid boundary patch
      unsigned int closestIndexId = ITKHelpers::ClosestIndexId(boundaryIndices, targetPixelIndex);
      itk::Index<2> closestBoundaryPatchCenter = boundaryIndices[closestIndexId];
      itk::ImageRegion<2> closestBoundaryPatchRegion =
            ITKHelpers::GetRegionInRadiusAroundPixel(closestBoundaryPatchCenter,
                                                      this->PatchRadius);

      if(!this->Image->GetLargestPossibleRegion().IsInside(closestBoundaryPatchRegion))
      {
        continue;
      }

      Match match;
      match.Region = closestBoundaryPatchRegion;
      match.Score = this->PatchDistanceFunctor->Distance(closestBoundaryPatchRegion, currentRegion);
      this->Output->SetPixel(targetPixelIndex, match);
    }

    std::cout << "Finished BoundaryInit()." << internalRegion << std::endl;
  }
};

#endif
