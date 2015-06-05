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

#ifndef PATCHMATCH_HPP
#define PATCHMATCH_HPP

#include "PatchMatch.h"

// Submodules
#include <ITKHelpers/ITKHelpers.h>
#include <ITKHelpers/ITKTypeTraits.h>

#include <Mask/MaskOperations.h>

#include <PatchComparison/SSD.h>

#include <Histogram/Histogram.h>

// ITK
#include "itkImageRegionReverseIterator.h"

// STL
#include <algorithm>
#include <ctime>

// Custom
#include "PatchMatchHelpers.h"
#include "RandomSearch.h"

template<typename TImage, typename TPropagation, typename TRandomSearch>
void PatchMatch<TImage, TPropagation, TRandomSearch>::Compute()
{
  assert(this->PropagationFunctor);
  assert(this->RandomSearchFunctor);

  // If the NNField is not already initialized, initialize it
  if(this->NNField->GetLargestPossibleRegion() != this->Image->GetLargestPossibleRegion())
  {
    RandomlyInitializeNNField();
  }

  this->RandomSearchFunctor->SetValidPatchCentersImage(this->ValidPatchCentersImage);
  this->RandomSearchFunctor->SetPixelsToProcess(this->TargetPixels);

  // For the number of iterations specified, perform the appropriate propagation and then a random search
  for(unsigned int iteration = 0; iteration < this->Iterations; ++iteration)
  {
    std::cout << "PatchMatch iteration " << iteration << std::endl;

    // We can propagate before random search because we are hoping the the random initialization gave us something good enough to propagate
    std::cout << "PatchMatch: Propagating..." << std::endl;
    this->PropagationFunctor->Propagate(this->NNField);

    UpdatedSignal(this->NNField);

    PatchMatchHelpers::WriteNNField(this->NNField.GetPointer(),
                                    Helpers::GetSequentialFileName("AfterPropagation", iteration, "mha"));

    std::cout << "PatchMatch: Random searching..." << std::endl;
    this->RandomSearchFunctor->Search(this->NNField);

    UpdatedSignal(this->NNField);

    PatchMatchHelpers::WriteNNField(this->NNField.GetPointer(),
                                    Helpers::GetSequentialFileName("AfterRandomSearch", iteration, "mha"));

    { // Debug only
    std::string sequentialFileName = Helpers::GetSequentialFileName("PatchMatch", iteration, "mha", 2);
    PatchMatchHelpers::WriteNNField(this->NNField.GetPointer(), sequentialFileName);
    }
  } // end iteration loop

  std::cout << "PatchMatch finished." << std::endl;
}

template<typename TImage, typename TPropagation, typename TRandomSearch>
void PatchMatch<TImage, TPropagation, TRandomSearch>::RandomlyInitializeNNField()
{
    itk::ImageRegion<2> internalRegion = ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(),
                                              this->PatchRadius);

    this->NNField->SetRegions(this->Image->GetLargestPossibleRegion());
    this->NNField->Allocate();

    itk::ImageRegionIteratorWithIndex<NNFieldType> nnFieldIterator(this->NNField, internalRegion);

    while(!nnFieldIterator.IsAtEnd())
    {
      itk::ImageRegion<2> targetRegion = ITKHelpers::GetRegionInRadiusAroundPixel(nnFieldIterator.GetIndex(), this->PatchRadius);

      itk::ImageRegion<2> randomRegion = PatchMatchHelpers::GetRandomRegionInRegion(internalRegion, this->PatchRadius);
      Match randomMatch;
      randomMatch.SetRegion(randomRegion);
      randomMatch.SetScore(this->RandomSearchFunctor->GetPatchDistanceFunctor()->Distance(randomRegion, targetRegion));

      nnFieldIterator.Set(randomMatch);
      ++nnFieldIterator;
    }
}

template<typename TImage, typename TPropagation, typename TRandomSearch>
void PatchMatch<TImage, TPropagation, TRandomSearch>::CorrectValidPatchCentersImage()
{
    itk::ImageRegion<2> internalRegion = ITKHelpers::GetInternalRegion(this->Image->GetLargestPossibleRegion(),
                                              this->PatchRadius);

    itk::ImageRegionIteratorWithIndex<BoolImageType> boolImageIterator(this->ValidPatchCentersImage, internalRegion);

    while(!boolImageIterator.IsAtEnd())
    {
      if(boolImageIterator.Get()) // if the pixel is marked as valid
      {
        itk::ImageRegion<2> queryRegion =
              ITKHelpers::GetRegionInRadiusAroundPixel(boolImageIterator.GetIndex(), this->PatchRadius);

        if(!internalRegion.IsInside(queryRegion))
        {
          boolImageIterator.Set(false);
        }
      }

      ++boolImageIterator;
    }
}

#endif
