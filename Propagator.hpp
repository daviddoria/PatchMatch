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

#ifndef Propagator_HPP
#define Propagator_HPP

Propagator::Propagator() : PatchRadius(0)
{
}

template <typename TPatchDistanceFunctor, typename TNeighborFunctor,
          typename TProcessFunctor, typename TAcceptanceTest>
void Propagator::
Propagate(NNFieldType* const nnField, TPatchDistanceFunctor* patchDistanceFunctor,
          TNeighborFunctor* neighborFunctor,
          TProcessFunctor* processFunctor, TAcceptanceTest* acceptanceTest)
{
  assert(neighborFunctor);
  assert(processFunctor);
  assert(acceptanceTest);

  itk::ImageRegion<2> region = nnField->GetLargestPossibleRegion();

  assert(nnField->GetLargestPossibleRegion().GetSize()[0] > 0); // An initialization must be provided

  std::vector<itk::Index<2> > targetPixels = processFunctor->GetPixelsToProcess();

  unsigned int propagatedPixels = 0;

  // Don't process the pixels we don't want to process
  targetPixels.erase(std::remove_if(targetPixels.begin(), targetPixels.end(),
                  [processFunctor](const itk::Index<2>& queryPixel)
                  {
                    return !processFunctor->ShouldProcess(queryPixel);
                  }),
                  targetPixels.end());

  // Don't process the pixels that already have an exact match.
  // When using PatchMatch for inpainting, most of the NN-field will be an exact match.
  // We don't have to search anymore once the exact match is found.
  targetPixels.erase(std::remove_if(targetPixels.begin(), targetPixels.end(),
                  [nnField](const itk::Index<2>& queryPixel)
                  {
                    return nnField->GetPixel(queryPixel).Score == 0;
                  }),
                  targetPixels.end());

  std::cout << "Propagation(): There are " << targetPixels.size()
            << " pixels that would like to be propagated to." << std::endl;

  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
    itk::Index<2> targetPixel = targetPixels[targetPixelId];

    itk::ImageRegion<2> targetRegion =
          ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->PatchRadius);

    if(!region.IsInside(targetRegion))
      {
        std::cerr << "targetRegion " << targetRegion << " is outside of the image." << std::endl;
        continue;
      }

    std::vector<itk::Index<2> > potentialPropagationPixels = neighborFunctor->GetNeighbors(targetPixel);

    bool propagated = false;
    for(size_t potentialPropagationPixelId = 0;
        potentialPropagationPixelId < potentialPropagationPixels.size();
        ++potentialPropagationPixelId)
    {
      itk::Index<2> potentialPropagationPixel = potentialPropagationPixels[potentialPropagationPixelId];

      itk::Offset<2> potentialPropagationPixelOffset = potentialPropagationPixel - targetPixel;

      //assert(this->Image->GetLargestPossibleRegion().IsInside(potentialPropagationPixel));
      if(!region.IsInside(potentialPropagationPixel))
      {
        //std::cerr << "Pixel " << potentialPropagationPixel << " is outside of the image." << std::endl;
        continue;
      }

      if(!nnField->GetPixel(potentialPropagationPixel).IsValid())
      {
        continue;
      }

      // The potential match is the opposite (hence the " - offset" in the following line)
      // of the offset of the neighbor. Consider the following case:
      // - We are at (4,4) and potentially propagating from (3,4)
      // - The best match to (3,4) is (10,10)
      // - potentialMatch should be (11,10), because since the current pixel is 1 to the right
      // of the neighbor, we need to consider the patch one to the right of the neighbors best match
      itk::Index<2> currentNearestNeighbor =
        ITKHelpers::GetRegionCenter(nnField->GetPixel(potentialPropagationPixel).Region);
      itk::Index<2> potentialMatchPixel = currentNearestNeighbor - potentialPropagationPixelOffset;

      itk::ImageRegion<2> potentialMatchRegion =
            ITKHelpers::GetRegionInRadiusAroundPixel(potentialMatchPixel, this->PatchRadius);

      if(!region.IsInside(potentialMatchRegion))
      {
        // do nothing - we don't want to propagate information that is not originally valid
        //std::cerr << "Cannot propagate from this source region " << potentialMatchRegion
        //          << " - it is outside of the image!" << std::endl;
      }
      else
      {
        float distance = patchDistanceFunctor->Distance(potentialMatchRegion, targetRegion);

        Match potentialMatch;
        potentialMatch.Region = potentialMatchRegion;
        potentialMatch.Score = distance;
        potentialMatch.Verified = true;

        //float oldScore = this->Output->GetPixel(targetRegionCenter).Score; // For debugging only
        //bool better = AddIfBetter(targetRegionCenter, potentialMatch);

        Match currentMatch = nnField->GetPixel(targetPixel);

        if(acceptanceTest->IsBetter(targetRegion, nnField->GetPixel(targetPixel), potentialMatch))
        {
          nnField->SetPixel(targetPixel, potentialMatch);
          propagated = true;
        }
        else
        {
          //std::cerr << "Acceptance test failed!" << std::endl;
        }

      } // end else source region valid
    } // end loop over potentialPropagationPixels

    if(propagated)
    {
      propagatedPixels++;
    }
    {
      //std::cerr << "Failed to propagate to " << targetPixel << std::endl;
    }

//     { // Debug only
//     std::string sequentialFileName = Helpers::GetSequentialFileName("PatchMatch_Propagation", targetPixelId, "mha");
//     PatchMatchHelpers::WriteNNField(temp.GetPointer(), sequentialFileName);
//     }
  } // end loop over target pixels

  std::cout << "Propagation() propagated " << propagatedPixels << " pixels." << std::endl;
}

#endif
