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

#include "Propagator.h"

#include <algorithm>

#include "itkImageRegionIteratorWithIndex.h"

template <typename TPatchDistanceFunctor>
unsigned int Propagator<TPatchDistanceFunctor>::
Propagate(NNFieldType* const nnField)
{
  assert(this->PatchDistanceFunctor);

  itk::ImageRegion<2> internalRegion = ITKHelpers::GetInternalRegion(nnField->GetLargestPossibleRegion(), this->PatchRadius);

  std::vector<itk::Index<2> > targetPixels = GetTraversalPixels(internalRegion);

//  std::cout << "Propagation(): There are " << targetPixels.size()
//            << " pixels that would like to be processed." << std::endl;

  unsigned int numberOfPropagatedPixels = 0;

  for(size_t targetPixelId = 0; targetPixelId < targetPixels.size(); ++targetPixelId)
  {
    itk::Index<2> targetPixel = targetPixels[targetPixelId];
    //ProcessPixelSignal(targetPixel);

    itk::ImageRegion<2> targetRegion =
          ITKHelpers::GetRegionInRadiusAroundPixel(targetPixel, this->PatchRadius);

    std::vector<itk::Offset<2> > propagationOffsets = GetPropagationOffsets();

    bool propagated = false;
    for(size_t propagationOffsetId = 0;
        propagationOffsetId < propagationOffsets.size();
        ++propagationOffsetId)
    {
      itk::Offset<2> propagationOffset = propagationOffsets[propagationOffsetId];

      //assert(this->Image->GetLargestPossibleRegion().IsInside(potentialPropagationPixel));
//      if(!fullRegion.IsInside(potentialPropagationPixel))
//      {
//        // This check should be done in the NeighborFunctor
//        //std::cerr << "Pixel " << potentialPropagationPixel << " is outside of the image." << std::endl;
//        continue;
//      }

      // The potential match is the opposite (hence the " - offset" in the following line)
      // of the offset of the neighbor. Consider the following case:
      // - We are at (4,4) and potentially propagating from (3,4)
      // - The best match to (3,4) is (10,10)
      // - potentialMatch should be (11,10), because since the current pixel is 1 to the right
      // of the neighbor, we need to consider the patch one to the right of the neighbors best match

      itk::Index<2> nnFieldLocation = targetPixel + propagationOffset;

      if(!internalRegion.IsInside(nnFieldLocation))
      {
          continue; // We don't want to propagate information from outside of the
                    // viable NN field region
      }

      NNFieldType::PixelType nnFieldPixel = nnField->GetPixel(nnFieldLocation);
      itk::Index<2> bestMatchPixel =
        ITKHelpers::GetRegionCenter(nnFieldPixel.GetRegion());

      itk::Index<2> potentialMatchPixel = bestMatchPixel - propagationOffset;

      if(!internalRegion.IsInside(potentialMatchPixel))
      {
          continue; // We don't want to propagate information from outside of the
                    // viable NN field region
      }

      itk::ImageRegion<2> potentialMatchRegion =
            ITKHelpers::GetRegionInRadiusAroundPixel(potentialMatchPixel, this->PatchRadius);


      float distance = this->PatchDistanceFunctor->Distance(potentialMatchRegion, targetRegion);

      Match potentialMatch;
      potentialMatch.SetRegion(potentialMatchRegion);
      potentialMatch.SetScore(distance);

      // If there were previous matches, add this one if it is better
      Match currentMatch = nnField->GetPixel(targetPixel);

      if(potentialMatch.GetScore() < currentMatch.GetScore())
      {
        nnField->SetPixel(targetPixel, potentialMatch);
      }

      //PropagatedSignal(nnField);
      propagated = true;

    } // end loop over potentialPropagationPixels

    if(propagated)
    {
      numberOfPropagatedPixels++;
    }

  } // end loop over target pixels

  // Reverse the propagation for the next iteration
  this->Forward = !this->Forward;

  //std::cout << "Propagation() propagated " << propagatedPixels << " pixels." << std::endl;
  //std::cout << "AcceptanceTest failed " << acceptanceTestFailed << std::endl;
  return numberOfPropagatedPixels;
}

template <typename TPatchDistanceFunctor>
std::vector<itk::Index<2> > Propagator<TPatchDistanceFunctor>::
GetTraversalPixels(const itk::ImageRegion<2>& region)
{
  std::vector<itk::Index<2> > traversalPixels;

  typedef itk::Image<int, 2> DummyImageType;
  DummyImageType::Pointer dummyImage = DummyImageType::New();
  dummyImage->SetRegions(region);
  dummyImage->Allocate();

  itk::ImageRegionIteratorWithIndex<DummyImageType> imageIterator(dummyImage, region);

  while(!imageIterator.IsAtEnd())
  {
    traversalPixels.push_back(imageIterator.GetIndex());
    ++imageIterator;
  }

  if(this->Forward == false)
  {
      std::reverse(traversalPixels.begin(), traversalPixels.end());
  }

  return traversalPixels;
}

template <typename TPatchDistanceFunctor>
std::vector<itk::Offset<2> > Propagator<TPatchDistanceFunctor>::
GetPropagationOffsets()
{
  std::vector<itk::Offset<2> > propagationOffsets;
  if(this->Forward)
  {
    itk::Offset<2> offset;
    offset[0] = -1;
    offset[1] = 0;
    propagationOffsets.push_back(offset);
    offset[0] = 0;
    offset[1] = -1;
    propagationOffsets.push_back(offset);
  }
  else
  {
    itk::Offset<2> offset;
    offset[0] = 1;
    offset[1] = 0;
    propagationOffsets.push_back(offset);
    offset[0] = 0;
    offset[1] = 1;
    propagationOffsets.push_back(offset);
  }
  return propagationOffsets;
}

#endif
