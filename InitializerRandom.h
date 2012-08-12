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

#ifndef InitializerRandom_H
#define InitializerRandom_H

#include "Initializer.h"

/** Set target pixels in the TargetMask to have random nearest neighbors. */
template <typename TPatchDistanceFunctor>
class InitializerRandom : public InitializerPatch
{
public:

  InitializerRandom() : PatchDistanceFunctor(NULL) {}

  /** Set target pixels (in the TargetMask) in 'initialization' to have random nearest neighbors.
    * Do not modify other pixles in 'initialization'.*/
  virtual void Initialize(itk::Image<Match, 2>* const initialization)
  {
    assert(this->PatchDistanceFunctor);
    assert(initialization);

    itk::ImageRegion<2> region = initialization->GetLargestPossibleRegion();

    itk::ImageRegion<2> internalRegion =
              ITKHelpers::GetInternalRegion(region, this->PatchRadius);

    std::vector<itk::ImageRegion<2> > validSourceRegions =
          MaskOperations::GetAllFullyValidRegions(this->SourceMask, internalRegion, this->PatchRadius);

    if(validSourceRegions.size() == 0)
    {
      throw std::runtime_error("PatchMatch: No valid source regions!");
    }

    std::vector<itk::Index<2> > targetPixels = this->TargetMask->GetValidPixels();
    std::cout << "RandomInit: There are : " << targetPixels.size() << " target pixels." << std::endl;
    for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
    {
      itk::ImageRegion<2> targetRegion =
        ITKHelpers::GetRegionInRadiusAroundPixel(targetPixels[targetPixelId], this->PatchRadius);
      if(!region.IsInside(targetRegion))
      {
        continue;
      }

      itk::Index<2> targetPixel = targetPixels[targetPixelId];
      if(initialization->GetPixel(targetPixel).IsValid())
      {
        continue;
      }
      unsigned int randomSourceRegionId = Helpers::RandomInt(0, validSourceRegions.size() - 1);
      itk::ImageRegion<2> randomValidRegion = validSourceRegions[randomSourceRegionId];

      Match randomMatch;
      randomMatch.Region = randomValidRegion;
      randomMatch.Score = this->PatchDistanceFunctor->Distance(randomValidRegion, targetRegion);
      randomMatch.Verified = false;

      initialization->SetPixel(targetPixel, randomMatch);
    }

    //std::cout << "Finished RandomInit." << internalRegion << std::endl;
  }
  
  void SetPatchDistanceFunctor(TPatchDistanceFunctor* const patchDistanceFunctor)
  {
    this->PatchDistanceFunctor = patchDistanceFunctor;
  }

protected:

  TPatchDistanceFunctor* PatchDistanceFunctor;
};

#endif

